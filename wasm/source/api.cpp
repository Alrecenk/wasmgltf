#include "Variant.h"
#include "GLTF.h"
#include <stdlib.h> 
#include <stdio.h>
#include <map>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include "glm/vec3.hpp"

using std::vector;
using std::string;
using std::map;
using std::pair;
using glm::vec3;
using glm::mat4;

// Outermost API holds a global reference to the core data model
GLTF model_global;
int model_global_id = 1;

Variant ret ; // A long lived variant used to hold data being returned to webassembly
byte* packet_ptr ; // location ofr data passed as function parameters and returns


int selected_animation = -1;
std::chrono::high_resolution_clock::time_point animation_start_time;

byte* pack(std::map<std::string, Variant> packet){
    ret = Variant(packet);
    memcpy(packet_ptr, ret.ptr, ret.getSize()); // TODO figure out how to avoid this copy
    return packet_ptr ;
}

// Convenience function that sets the return Variant to an empty map and returns it.
// "return emptyReturn();" can be used in any front-end function to return a valid empty object
byte* emptyReturn() {
    map<string, Variant> ret_map;
    return pack(ret_map);
}


std::chrono::high_resolution_clock::time_point now(){
    return std::chrono::high_resolution_clock::now();
} 



int millisBetween(std::chrono::high_resolution_clock::time_point start, std::chrono::high_resolution_clock::time_point end){
    return (int)(std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count()*1000);
}

extern "C" { // Prevents C++ from mangling the exported name apparently


void setPacketPointer(byte* p){
    printf("Packet Pointer allocated at: %ld\n", (long)p);
    packet_ptr = p;
    map<string, Variant> ret_map;
    pack(ret_map);
}


// Wrappers to model functions applied to model global are made available to callers
// Expects an object with vertices and faces
byte* setModel(byte* ptr){
    auto obj = Variant::deserializeObject(ptr);
    byte* byte_array = obj["data"].getByteArray();
    int num_bytes = obj["data"].getArrayLength();
    model_global.setModel(byte_array, num_bytes);

    float size = 0 ;
    vec3 center(0,0,0);
    for(int k=0;k<3;k++){
        center[k] = (model_global.max[k]+ model_global.min[k])*0.5f ;
        size = fmax(size , abs(model_global.max[k]-center[k]));
        size = fmax(size , abs(model_global.min[k]-center[k]));
        
    }
    
    /*
    for(int k=0;k<model_global.vertices.size();k++){
        model_global.vertices[k].position = (model_global.vertices[k].position-center)*(1.0f/size);
    }*/
    

    model_global.transform  = glm::scale(mat4(1), {(1.0f/size),(1.0f/size),(1.0f/size)});
    model_global.transform  = glm::translate(model_global.transform, center*-1.0f);
    
    model_global.computeNodeMatrices();
    model_global.applyTransforms();
    
    //printf("Zoom:%f\n", zoom);
    map<string, Variant> ret_map;
    ret_map["center"] = Variant(center);
    ret_map["size"] = Variant(size);
    return pack(ret_map);
}



byte* getUpdatedBuffers(byte* ptr){

    if(selected_animation >= 0){
        auto& animation = model_global.animations[selected_animation];
        float time = millisBetween(animation_start_time, now()) / 1000.0f;
            while(time > animation.duration){
                time-= animation.duration;
            }
        model_global.animate(animation,time);
        //model_global.applyTransforms();
        //stopped = true;
    }

    map<string,Variant> buffers;
    //st = now();
    if(model_global.position_changed || model_global.model_changed || model_global.bones_changed){
        for(auto const & [material_id, mat]: model_global.materials){
            std::stringstream ss;
            ss << material_id;
            string s_id = ss.str();
            buffers[s_id] = model_global.getChangedBuffer(material_id) ;
        }
        model_global.position_changed = false;
        model_global.model_changed = false;
        model_global.bones_changed = false;
        /*
        for(int k=0;k<model_global.triangles.size();k++){
            if(model_global.materials.find(model_global.triangles[k].material) == model_global.materials.end()){
                printf("triangle has material not in materials : %d!\n", model_global.triangles[k].material);
            }
        }*/
            
    }
    //ms = millisBetween(st, now());
    //printf("api get all buffers: %d ms\n", ms);
    return pack(buffers) ;
}

//expects an object with p and v, retruns a serialized single float for t
byte* rayTrace(byte* ptr){
    auto obj = Variant::deserializeObject(ptr);
    vec3 p = obj["p"].getVec3() ;
    vec3 v = obj["v"].getVec3();
    float t = model_global.rayTrace(p, v);
    map<string, Variant> ret_map;
    ret_map["t"] = Variant(t);
    ret_map["x"] = Variant(p+v*t);
    ret_map["index"] = Variant(model_global.last_traced_tri);
    return pack(ret_map);
}

byte* scan(byte* ptr){
    auto obj = Variant::deserializeObject(ptr);
    vec3 p = obj["p"].getVec3() ;
    vec3 v = obj["v"].getVec3();
    float t = model_global.rayTrace(p, v);
    map<string, Variant> ret_map;
    ret_map["t"] = Variant(t);
    ret_map["x"] = Variant(p+v*t);
    ret_map["index"] = Variant(model_global.last_traced_tri);

    if(model_global.last_traced_tri != -1){
        GLTF::Triangle tri = model_global.triangles[model_global.last_traced_tri];
        int material_id = tri.material;
        GLTF::Material mat = model_global.materials[tri.material];
        printf("triangle : %d, material: %d \n" ,model_global.last_traced_tri, material_id);
        if(model_global.json["materials"][material_id].defined()){
            model_global.json["materials"][material_id].printFormatted();
        }
        
        GLTF::Vertex& v = model_global.vertices[tri.A];
        glm::ivec4 joints = v.joints;
        for(int k=0;k<4;k++){
            int n = v.joints[k];
            string name = model_global.nodes[n].name;
            float w = v.weights[k] ;
            if(w > 0){
                printf("Joint[%d] = %d (%s)  weight: %f\n", k, n, name.c_str(), w);
            }
        }

        //printf("Local Position: %f, %f, %f \n" , v.base_position.x, v.base_position.y, v.base_position.z);
    }
    return emptyReturn();
}

byte* nextAnimation(byte* ptr){
    selected_animation = selected_animation+1;
    if(selected_animation >= model_global.animations.size()){
        selected_animation = -1;
        model_global.setBasePose();
        model_global.computeNodeMatrices();
        //model_global.applyTransforms();
    }
    animation_start_time = now();
    return emptyReturn();
}

//Allocates an array of integers of a given size and returns a pointer to it.
// Note: this is intended for testing JS to C++ pointer moves and memory limits and LEAKS MEMORY if not cleared on the JS side.
byte* testAllocate(byte* ptr){
    auto obj = Variant::deserializeObject(ptr);
    int size = obj["size"].getInt();
    int num = obj["num"].getInt();

    int* array = new int[size];
    for(int k=0;k<size;k++){
        array[k] = num + k;
    }
    printf("C++ array ptr: %ld\n", (long)array);
    map<string, Variant> ret_map;
    ret_map["ptr"] = Variant((int)array);
    ret = Variant(ret_map);
    printf("C++ ret ptr: %ld\n", (long)ret.ptr);
    auto obj2 = Variant::deserializeObject(ret.ptr);
    printf("C++ deserialized array ptr: %ld\n", (long)obj2["ptr"].getInt());
    return pack(ret_map);

}

}// end extern C
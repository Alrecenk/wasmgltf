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
#include "TableReader.h"

using std::vector;
using std::string;
using std::map;
using std::pair;
using glm::vec3;
using glm::vec4;
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

// Note: Javascript deserializer expects string objects
// This is only intended for server interfaces that don't deserialize in Javascript
byte* pack(Variant packet){
    memcpy(packet_ptr, packet.ptr, packet.getSize()); // TODO figure out how to avoid this copy
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

    
    auto start_time = now();
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

    int millis = millisBetween(start_time, now());
    printf("Total model load time: %d ms\n", millis);

    return pack(ret_map);
}



byte* getUpdatedBuffers(byte* ptr){

    if(selected_animation >= 0){
        auto& animation = model_global.animations[selected_animation];
        float time = millisBetween(animation_start_time, now()) / 1000.0f;
            if(time > animation.duration){
                time = 0 ;
                animation_start_time = now();
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

    model_global.applyTransforms(); // Get current animated coordinates on CPU
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
    if(millisBetween(animation_start_time, now()) < 50){
        return emptyReturn();
    }
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


// Returns the pending table requests that require a network request to fetch
// Items which are already in the cache are given to objects when this is called
//Note: this returns a pointer to VARIANT_ARRAY data but not the type, so it cannot be deserialized by default methods that assume an OBJECT type
byte* getTableNetworkRequest(byte* nothing) {
    TableReader::serveCachedRequests();
    vector<string> requests = TableReader::getRequestedKeys();
    vector<Variant> network_request;
    network_request.reserve(requests.size());
    for (const auto &key : requests) {
        // emplace back constructs Variants from strings
        network_request.emplace_back(key);
    }
    return pack(Variant(network_request));
}

//Caches and Distributes data from a network table data response
byte* distributeTableNetworkData(byte* data_ptr) {
    map<string, Variant> data = Variant::deserializeObject(data_ptr);
    TableReader::receiveTableData(data);
    return emptyReturn();
}

byte* createPin(byte* ptr) {
    auto obj = Variant::deserializeObject(ptr);
    vec3 p = obj["p"].getVec3() ;
    vec3 v = obj["v"].getVec3();
    string name = obj["name"].getString();

    model_global.applyTransforms(); // Get current animated coordinates on CPU
    float t = model_global.rayTrace(p, v);

    if(model_global.last_traced_tri != -1){
        GLTF::Triangle tri = model_global.triangles[model_global.last_traced_tri];        
        GLTF::Vertex& vert = model_global.vertices[tri.A];
        glm::ivec4 joints = vert.joints;
        float max_w = -1.0f ;
        int bone = -1;
        for(int k=0;k<4;k++){
            float w = vert.weights[k] ;
            if(w > max_w){
                max_w = w ;
                bone = vert.joints[k] ;
            }
        }
        //TODO make pin
        vec3 global = p + v * t;
        // glm::inverse(model_global.transform) 
        vec3 mesh_space =   glm::inverse(model_global.nodes[bone].transform) * vec4(global,1) ;
        vec3 local = model_global.nodes[bone].mesh_to_bone * vec4(mesh_space,1) ; // TODO
        
        //vec3 actual =  model_global.nodes[bone].transform * (model_global.nodes[bone].bone_to_mesh * vec4(local,1));

        model_global.createPin(name, bone, local, 1.0f);

        /*
        printf("global: %f, %f, %f\n", global.x, global.y, global.z);
        printf("actual: %f, %f, %f\n", actual.x, actual.y, actual.z);
        printf("Mesh: %f, %f, %f\n", mesh_space.x, mesh_space.y, mesh_space.z);
        printf("Local: %f, %f, %f\n", local.x, local.y, local.z);
        printf("Vert: %f, %f, %f\n", vert.position.x, vert.position.y, vert.position.z);
        printf("Vert Transformed: %f, %f, %f\n", vert.transformed_position.x, vert.transformed_position.y, vert.transformed_position.z);
        

        double error = model_global.error(model_global.getX());
        printf("Error: %f\n", error);

        printf("Pin '%s' created for %s.\n" , name.c_str(), model_global.nodes[bone].name.c_str());
        */
    }

    return emptyReturn();

}

byte* setPinTarget(byte* ptr) {
    auto obj = Variant::deserializeObject(ptr);
    vec3 p = obj["p"].getVec3() ;
    vec3 v = obj["v"].getVec3();
    string name = obj["name"].getString();

    GLTF::Pin& pin = model_global.pins[name] ;
    vec3 current = model_global.nodes[pin.bone].transform * ( model_global.nodes[pin.bone].bone_to_mesh * vec4(pin.local_point,1));

    // Pull toward the closest point on the mouse ray
    vec3 target = p + v * (glm::dot(current-p, v) / glm::dot(v,v)) ;
    //printf("Pulling %s to (%f, %f, %f).\n", name.c_str(), target.x, target.y, target.z);
    model_global.setPinTarget(name, target);

    model_global.applyPins();

    return emptyReturn();
}

byte* deletePin(byte* ptr) {
    auto obj = Variant::deserializeObject(ptr);
    string name = obj["name"].getString();
    model_global.deletePin(name);
    //printf("Pin '%s' deleted.\n" , name.c_str());
    return emptyReturn();
}


}// end extern C
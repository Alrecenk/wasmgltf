#include "GLTF.h"
#include "Variant.h"

#include <math.h>
#include <map>
#include <vector>
#include <string>
#include <limits>
#include <stdlib.h>
#include <queue>

using std::string;
using std::vector;
using std::map;
using glm::vec3;
using glm::vec4;
using glm::mat4;
typedef GLTF::Triangle Triangle;
using std::string;
using std::queue;


// Constructor
GLTF::GLTF(){
    this->num_vertices = 0;
    this->num_triangles = 0;
    this->color_changed = false;
    this->vertices_changed = false;
}

//Destructor
GLTF::~GLTF(){
}

// Returns a Variant of 3 vec3's'for each each triangle dereferenced
// Used to get arrays for displaying with a simple shader
Variant GLTF::getFloatBuffer(std::vector<glm::vec3>& point_list){
    Variant buffer;
    //TODO this access pattern prevents these Variant members from being private 
    // but using the constructor forces a copy that isn't necesarry 
    // Maybe here should be an array constructor that just takes type and size
    // and then the get array provides a shallow pointer that can't be freed?
    buffer.type_ = Variant::FLOAT_ARRAY;
    buffer.ptr = (byte*)malloc(4 + this->num_triangles * 9 * sizeof(float));
    
    *((int*)buffer.ptr) = this->num_triangles * 9 ;// number of floats in array
    float* buffer_array =  (float*)(buffer.ptr+4) ; // pointer to start of float array
    for(int k=0;k<this->num_triangles;k++){
        Triangle& t = this->triangles[k];
        int k9 = k*9; // does this really matter?
        // A
        vec3& A = point_list[t.A];
        buffer_array[k9] = A.x;
        buffer_array[k9+1] = A.y;
        buffer_array[k9+2] = A.z;
        // B
        vec3& B = point_list[t.B];
        buffer_array[k9+3] = B.x;
        buffer_array[k9+4] = B.y;
        buffer_array[k9+5] = B.z;
        // C
        vec3& C = point_list[t.C];
        buffer_array[k9+6] = C.x;
        buffer_array[k9+7] = C.y;
        buffer_array[k9+8] = C.z;
    }
    return buffer ;
}

// Returns a Variant of openGL triangle buffers for displaying this mesh_ in world_ space
// result["position"] = float array of triangle vertices in order (Ax,Ay,Az,Bx,By,Bz,Cx,Cy,Cz)
// result["normal] = same format for vertex normals
// result["color"] = same format for colors but RGB floats from 0 to 1
// result["vertices"] = number of vertices
Variant GLTF::getChangedBuffers(){
    std::map<string, Variant> buffers;
    if(vertices_changed){
        buffers["position"] = this->getFloatBuffer(this->vertices);
        buffers["normal"] = this->getFloatBuffer(this->normals);
        vertices_changed = false;
    }
    if(color_changed){
        buffers["color"] = this->getFloatBuffer(this->colors);
        color_changed = false;
    }
    buffers["vertices"] = Variant(num_triangles * 3);
    return Variant(buffers);
}

void GLTF::setModel(const byte* data, int data_length){
    
    vector<vec3> new_vertices;
    vector<Triangle> new_triangles;

    printf ("num bytes: %d \n", data_length);
    printf("GLB Magic: %u  JSON_CHUNK: %u\n", 0x46546C67, 0x4E4F534A);
    uint magic_num = *(uint *)data;

    if(magic_num == 0x46546C67){ // check if GLTF
        uint version = *(int *)(data + 4);
        uint total_length = *(int *)(data + 8);
        printf("GLB file detected. Version : %u  Data Length: %u\n", version, total_length);
        printf("File Length %d\n", data_length);

        uint JSON_length = *((uint *) (data + 12));// in bytes, not necessarily characters
        uint first_chunk_type = *((uint *) (data + 16));
        
        if(first_chunk_type == 0x4E4F534A){
            printf("JSON chunk found!\n");

            string header = string((char *) (data + 20), JSON_length);
            //Variant::printJSON(s);
            json = Variant::parseJSON(header);
            json.printFormatted();
            int bin_chunk_start = 20 + JSON_length ;
            
            if(bin_chunk_start %4 != 0){
                bin_chunk_start += 4 - (bin_chunk_start %4);
            }
            uint bin_length = *((uint*)(data+bin_chunk_start)) ;
            uint second_chunk_type = *((uint*)(data+bin_chunk_start+4)) ;
            if(second_chunk_type == 0x004E4942){
                printf("Bin chunk found!\n");
                printf("Bin size: %d \n", bin_length);
                bin = Variant(data+bin_chunk_start+8, bin_length);
                int default_scene = 0 ;
                if(json["scene"].defined()){
                    default_scene = json["scene"].getInt() ;
                }
                addScene(new_vertices, new_triangles, default_scene, json, bin) ;

            }else{
                printf("Bin chunk not found after json !(got %d)\n", second_chunk_type);
            }

           
        }else{
            printf("first chunk not json: %u\n", first_chunk_type);
        }

    }else{
        printf("Not a GLB file! %d != %d\n", magic_num, 0x46546C67);
    }
    
    printf("Total triangles: %d\n",(int) new_triangles.size());
    setModel(new_vertices, new_triangles);

}

GLTF::Accessor GLTF::access(int accessor_id, const Variant& json, const Variant& bin){

    
    map<string,int> TYPE_LENGTH = {{"SCALAR",1},{"VEC2",2},{"VEC3",3},{"VEC4",4},{"MAT2",4},{"MAT3",9},{"MAT4",16}} ; // TODO static const
    map<int,int> COMPONENT_LENGTH = {{5120, 1},{5121, 1},{5122, 2},{5123, 2},{5125, 4},{5126, 4}}; // TODO static const

    
    auto accessor = json["accessors"][accessor_id];
    //printf("accessor:\n");
    //accessor.printFormatted();
    string type = accessor["type"].getString();
    uint c_type = (uint)(accessor["componentType"].getInt());
    int count = accessor["count"].getInt();
    int element_length = TYPE_LENGTH[type] * COMPONENT_LENGTH[c_type];

    auto view = json["bufferViews"][accessor["bufferView"]];
    int offset = 0 ;
    if(view["byteOffset"].defined()){
        offset = view["byteOffset"].getInt();
    }
    if(accessor["byteOffset"].defined()){
        offset += accessor["byteOffset"].getInt();
    }
    int byteLength = view["byteLength"].getInt();
    //printf("view:\n");
    //view.printFormatted();
    int stride = element_length;
    if(view["byteStride"].defined()){
        stride = view["byteStride"].getInt();
    }

    GLTF::Accessor result = {type, c_type, Variant()};
    result.data.ptr = (byte*)malloc(4 + element_length*count);
    ((int*)result.data.ptr)[0] = count * TYPE_LENGTH[type]; 
    for(int k=0;k<count;k++){
        memcpy(result.data.ptr + 4 +k*element_length, bin.ptr + 4 + offset + k*stride, element_length);
    }

    if(c_type == 5126){ // 32 bit float
        result.data.type_ = Variant::FLOAT_ARRAY;
    }else if(c_type == 5120 || 5121){ // signed and unsigned byte
        result.data.type_ = Variant::BYTE_ARRAY ;
    }else if(c_type == 5122 || c_type == 5123){ // shorts
        result.data.type_ = Variant::SHORT_ARRAY ;
    }else if(c_type == 5125){ // unsigned int
        result.data.type_ = Variant::INT_ARRAY ;
    }else{
        //printf("unrecognized accessor component type, behavior undefined!\n");
        result.data.type_ = Variant::BYTE_ARRAY ;
    }

    return result ;
}



//TODO consider using quaternion down the hierarchy recursion instead of mat4 for better precision/speed.
void GLTF::addPrimitive(std::vector<glm::vec3>& vertices, std::vector<Triangle>& triangles,
                        const Variant& primitive, const glm::mat4& transform, const Variant& json, const Variant& bin){
    //printf("Adding primitive:\n");
    primitive.printFormatted();
    if(primitive["mode"].defined() && primitive["mode"].getInt() != 4){
        printf("Primitive mode %d not implemented yet. Skipping.\n", primitive["mode"].getInt()); // TODO
        return ;
    }

    
    GLTF::Accessor pa = GLTF::access(primitive["attributes"]["POSITION"].getInt(), json, bin);
    if(pa.type != "VEC3" || pa.component_type != 5126){
        printf("expected vec3 floats for position, aborting.\n");
        return ;
    }

    //pa.data.printFormatted();

    int num_vertices = pa.data.getArrayLength()/3;
    //printf("num_vertices: %d \n", num_vertices);
    float* point_data = pa.data.getFloatArray(); // still held by Variant, not a memory leak
    uint* index_data = nullptr; // this one you need to be careful
    int num_indices = 0 ;

    int start_vertices = vertices.size();
    //printf("Start vertices: %d \n", start_vertices); 

    if(primitive["indices"].defined()){
        //printf("Found indices!\n");
        GLTF::Accessor ia = GLTF::access(primitive["indices"].getInt(), json, bin);
        if(ia.type == "SCALAR"){
            if(ia.component_type == 5121){
                printf("unsigned byte indices, technically vlaid, but not yet implemented, aborting\n"); // TODO
                return ;
            }else if(ia.component_type == 5123){
                //printf("unsigned short indices\n");
                num_indices = ia.data.getArrayLength();
                short* shorts = ia.data.getShortArray();
                index_data = (uint*)malloc(4*num_indices);
                for(int k=0;k<num_indices;k++){
                    index_data[k] = (unsigned short)shorts[k];
                }
            }else if(ia.component_type == 5125){
                //printf("unsigned int indices\n");
                num_indices = ia.data.getArrayLength();
                int* ints = ia.data.getIntArray();
                index_data = (uint*)malloc(4*num_indices);
                for(int k=0;k<num_indices;k++){
                    index_data[k] = (unsigned int)ints[k];
                }
            }else{
                //printf("Indices are not a valid type ( %d)  aborting\n", ia.component_type);
                return ;
            }

            if(num_indices > 0){
                //printf("Found points and indices successfully!\n");
                
                for(int k=0;k<num_vertices;k++){
                    //printf("vertex: %f , %f , %f\n", point_data[3*k], point_data[3*k+1],point_data[3*k+2]);
                    vec3 v_local = vec3(point_data[3*k], point_data[3*k+1],point_data[3*k+2]) ;
                    vec4 v_global = transform*vec4(v_local,1);
                    //printf("global: %f , %f , %f\n", v_global.x, v_global.y, v_global.z);
                    vertices.push_back(vec3(v_global));
                }

                for(int k=0;k<num_indices;k+=3){
                    //printf("Triangle: %d , %d , %d\n", (int)index_data[k]+start_vertices, (int)index_data[k+1]+start_vertices,(int)index_data[k+2]+start_vertices);
                    triangles.push_back({(int)index_data[k]+start_vertices,
                                    (int)index_data[k+1]+start_vertices, 
                                    (int)index_data[k+2]+start_vertices});
                }

                free(index_data);
            }

        }else{
            printf("Indices are not scalar, aborting\n");
            return ;
        }
    }else{
        //printf("no indices found.\n"); //TODO
        for(int k=0;k<num_vertices;k++){
            //printf("vertex: %f , %f , %f\n", point_data[3*k], point_data[3*k+1],point_data[3*k+2]);
            vec3 v_local = vec3(point_data[3*k], point_data[3*k+1],point_data[3*k+2]) ;
            vec4 v_global = transform*vec4(v_local,1);
            //printf("global: %f , %f , %f\n", v_global.x, v_global.y, v_global.z);
            vertices.push_back(vec3(point_data[3*k], point_data[3*k+1],point_data[3*k+2]));
            if(k%3 == 2){
                triangles.push_back({k-2+start_vertices,
                                   k-1+start_vertices, 
                                    k+start_vertices});
            }
        }
    }
}

void GLTF::addMesh(std::vector<glm::vec3>& vertices, std::vector<Triangle>& triangles,
                   int mesh_id, const glm::mat4& transform, const Variant& json, const Variant& bin){
    printf("Adding mesh %d!\n", mesh_id);

    auto primitives = json["meshes"][mesh_id]["primitives"];
    for(int k=0;k<primitives.getArrayLength();k++){
        addPrimitive(vertices, triangles, primitives[k], transform, json, bin);
    }
}

void GLTF::addNode(std::vector<glm::vec3>& vertices, std::vector<Triangle>& triangles,
                   int node_id, const glm::mat4& transform, const Variant& json, const Variant& bin){
    printf("Adding node %d!\n", node_id);
    auto node = json["nodes"][node_id];

    mat4 new_transform = transform ;
    if(node["matrix"].defined()){
        printf("Node has matrix not yet implemented!\n");
        //new_transform *= M;
    }else{
        /*
        printf("Initial transform:\n");
            for(int k=0;k<4;k++){
                printf("[ %f, %f, %f, %f]\n", new_transform[k][0], new_transform[k][1], new_transform[k][2], new_transform[k][3]);
            }
            */

        Variant tv = node["translation"];
        if(tv.defined()){
            vec3 t = vec3(tv[0].getNumberAsFloat(), tv[1].getNumberAsFloat(), tv[2].getNumberAsFloat());
            new_transform = glm::translate(new_transform, t);
            /*
            printf("After translate:\n");
            for(int k=0;k<4;k++){
                printf("[ %f, %f, %f, %f]\n", new_transform[k][0], new_transform[k][1], new_transform[k][2], new_transform[k][3]);
            }
            */
        }
        Variant rv = node["rotation"] ;
        if(rv.defined()){
            // GLB is XYZW but GLM:quat is WXYZ
            glm::quat qrot(rv[3].getNumberAsFloat(), rv[0].getNumberAsFloat(), rv[1].getNumberAsFloat(), rv[2].getNumberAsFloat() );
            new_transform *= glm::mat4_cast(qrot);
            /*
            printf("After rotate:\n");
            for(int k=0;k<4;k++){
                printf("[ %f, %f, %f, %f]\n", new_transform[k][0], new_transform[k][1], new_transform[k][2], new_transform[k][3]);
            }
            */
        }
        Variant sv = node["scale"];
        if(sv.defined()){
            vec3 s = vec3(sv[0].getNumberAsFloat(), sv[1].getNumberAsFloat(), sv[2].getNumberAsFloat());
            new_transform = glm::scale(new_transform, s);
            /*
            printf("After scale:\n");
            for(int k=0;k<4;k++){
                printf("[ %f, %f, %f, %f]\n", new_transform[k][0], new_transform[k][1], new_transform[k][2], new_transform[k][3]);
            }
            */
        }

        
        
    }
    
    if(node["mesh"].defined()){
        int mesh_id = node["mesh"].getInt();
        addMesh(vertices, triangles, mesh_id, new_transform, json, bin);
    }
    if(node["children"].defined()){
        auto nodes = node["children"];
        for(int k=0;k<nodes.getArrayLength();k++){
            int node_id = nodes[k].getInt();
            addNode(vertices, triangles, node_id, new_transform, json, bin);
        }
    }
}

void GLTF::addScene(std::vector<glm::vec3>& vertices, std::vector<Triangle>& triangles,
                    int scene_id, const Variant& json, const Variant& bin){
    printf("Adding scene %d!\n", scene_id);
    auto nodes = json["scenes"][scene_id]["nodes"];
    glm::mat4 ident(1);
    for(int k=0;k<nodes.getArrayLength();k++){
        int node_id = nodes[k].getInt();
        addNode(vertices, triangles, node_id, ident, json, bin);
    }
}


// Compacts the given vertices and sets the model to them
void GLTF::setModel(const std::vector<vec3>& vertices, const std::vector<Triangle>& triangles){

    vector<vec3> new_vertices;
    vector<Triangle> new_triangles;

    map<int, int> unique_vertices ; // map hashes to new vertex indices

    for(int k=0;k<vertices.size();k++){
        int hash = hashVertex(vertices[k]);
        if(unique_vertices.find(hash) == unique_vertices.end()){
            unique_vertices[hash] = new_vertices.size() ;
            new_vertices.push_back(vertices[k]);
        }
    }
    for(int k=0;k<triangles.size();k++){
        new_triangles.push_back(Triangle{
            unique_vertices[hashVertex(vertices[triangles[k].A])],
            unique_vertices[hashVertex(vertices[triangles[k].B])],
            unique_vertices[hashVertex(vertices[triangles[k].C])]
        });
    }

    this->vertices_changed = true;
    this->color_changed = true;

    this->num_vertices = new_vertices.size();
    this->num_triangles = new_triangles.size();
    this->vertices = new_vertices;
    this->triangles = new_triangles;

     // Set all colors to white and Initialize Normals
    
    this->colors = vector<glm::vec3>();
    this->normals = vector<glm::vec3>();
    for(int k=0; k<this->num_vertices; k++){
        this->colors.push_back(vec3{1.0f, 1.0f, 1.0f}) ;
        this->normals.push_back(vec3{0.0f, 0.0f, 0.0f});
    }
    
    // get normals by summing up touching triangles
    for(int k=0; k < this->num_triangles; k++){
        Triangle& t = this->triangles[k] ;
        vec3 n = this->getNormal(t);
        this->normals[t.A] += n ;
        this->normals[t.B] += n ;
        this->normals[t.C] += n ;
    }
    //normalize normals
    for(int k=0;k<this->num_vertices;k++){
        this->normals[k] = glm::normalize(this->normals[k]) ;
    }
    
    //update AABB
    this->min = {9999999,9999999,9999999};
    this->max = {-9999999,-9999999,-9999999};
    for(int k=0;k<this->num_vertices; k++){
        auto &v = this->vertices[k];
        if(v.x < this->min.x)this->min.x = v.x;
        if(v.y < this->min.y)this->min.y = v.y;
        if(v.z < this->min.z)this->min.z = v.z;
        if(v.x > this->max.x)this->max.x = v.x;
        if(v.y > this->max.y)this->max.y = v.y;
        if(v.z > this->max.z)this->max.z = v.z;
    }
}

// hashes a vertex to allow duplicates to be detected and merged
int GLTF::hashVertex(vec3 v){
    return Variant(v).hash();
}

vec3 GLTF::getNormal(Triangle t){
    vec3 AB = this->vertices[t.B] - this->vertices[t.A];
    vec3 AC = this->vertices[t.C] - this->vertices[t.A];
    return glm::normalize(glm::cross(AB,AC));
}

// Given a ray in model space (p + v*t) return the t value of the nearest collision
// with the given triangle
// return negative if no collision
float EPSILON = 0.00001;
float GLTF::trace(Triangle tri, const vec3 &p, const vec3 &v){
    vector<vec3>& x = this->vertices ;
    //TODO use the vector library
    vec3 AB = {x[tri.B].x-x[tri.A].x,x[tri.B].y-x[tri.A].y,x[tri.B].z-x[tri.A].z};
    vec3 AC = {x[tri.C].x-x[tri.A].x,x[tri.C].y-x[tri.A].y,x[tri.C].z-x[tri.A].z};
    // h = v x AC
    vec3 h = {v.y*AC.z - v.z*AC.y,
              v.z*AC.x - v.x*AC.z,
              v.x*AC.y - v.y*AC.x};
    // a = AB . h
    float a = AB.x*h.x + AB.y*h.y + AB.z*h.z;
	if (a < EPSILON)
      return -1;
	float f = 1.0 / a;
    // s = p - A
    vec3 s = {p.x-x[tri.A].x,p.y-x[tri.A].y,p.z-x[tri.A].z};
    // u = s.h * f
    float u = (s.x*h.x+s.y*h.y+s.z*h.z) * f;
	if (u < 0 || u > 1)
      return -1;
	// q = s x AB
    vec3 q = {s.y*AB.z - s.z*AB.y,
              s.z*AB.x - s.x*AB.z,
              s.x*AB.y - s.y*AB.x};
    // w = v.q * f
    float w = (v.x*q.x+v.y*q.y+v.z*q.z) * f;
	if (w < 0 || u + w > 1)
      return -1;
    // t = AC.q * f
    float t = (AC.x*q.x+AC.y*q.y+AC.z*q.z) * f;
    if( t > EPSILON){
      return t;
    }else{
      return -1;
    }

}

// Given a ray in model space (p + v*t) return the t value of the nearest collision
// return negative if no collision
float GLTF::rayTrace(const vec3 &p, const vec3 &v){
    float min_t = std::numeric_limits<float>::max() ;
    for(int k=0;k<this->num_triangles;k++){
        float t = this->trace(this->triangles[k],p,v);
        if(t > 0 && t < min_t){
            min_t = t ;
        }
    }
    if(min_t < std::numeric_limits<float>::max() ){
        return min_t ;
    }return -1;
}

// Changes all vertices within radius of origin to the given color
void GLTF::paint(const vec3 &center, const float &radius, const vec3 &color){
    float r2 = radius*radius;
    for(int k=0; k<this->num_vertices; k++){
        vec3 d = {this->vertices[k].x-center.x,this->vertices[k].y-center.y,this->vertices[k].z-center.z};
        float d2 = d.x*d.x+d.y*d.y+d.z*d.z;
        if(d2<r2){
            // if actually changing
            if(this->colors[k].x != color.x || this->colors[k].y != color.y || this->colors[k].z != color.z ){
                this->colors[k] = color;
                this->color_changed = true;
            }
        }
    }
}
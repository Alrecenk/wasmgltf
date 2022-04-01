#include "GLTF.h"
#include "Variant.h"

#include <math.h>
#include <map>
#include <vector>
#include <string>
#include <limits>
#include <stdlib.h>
#include <queue>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::string;
using std::vector;
using std::map;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec4;
using glm::mat4;
typedef GLTF::Triangle Triangle;
using std::string;
using std::queue;


// Constructor
GLTF::GLTF(){
    this->position_changed = false;
    this->model_changed = false;
}

//Destructor
GLTF::~GLTF(){
}

// Returns a Variant of 3 vec3's'for each each triangle dereferenced
// Used to get arrays for displaying with a simple shader
Variant GLTF::getFloatBuffer(std::vector<glm::vec3>& point_list, int material){

    int num_triangles = 0 ;
    for(int k=0;k<this->triangles.size();k++){
        if(this->triangles[k].material == material){
            num_triangles++;
        }
    }

    //TODO this access pattern prevents these Variant members from being private 
    // but using the constructor forces a copy that isn't necesarry 
    // Maybe here should be an array constructor that just takes type and size
    // and then the get array provides a shallow pointer that can't be freed?
    Variant buffer;
    buffer.type_ = Variant::FLOAT_ARRAY;
    buffer.ptr = (byte*)malloc(4 + num_triangles * 9 * sizeof(float));
    
    *((int*)buffer.ptr) = num_triangles * 9 ;// number of floats in array
    float* buffer_array =  (float*)(buffer.ptr+4) ; // pointer to start of float array
    int j9 = 0 ;
    for(int k=0;k<this->triangles.size();k++){
        if(this->triangles[k].material == material){
            Triangle& t = this->triangles[k];
            // A
            vec3& A = point_list[t.A];
            buffer_array[j9] = A.x;
            buffer_array[j9+1] = A.y;
            buffer_array[j9+2] = A.z;
            // B
            vec3& B = point_list[t.B];
            buffer_array[j9+3] = B.x;
            buffer_array[j9+4] = B.y;
            buffer_array[j9+5] = B.z;
            // C
            vec3& C = point_list[t.C];
            buffer_array[j9+6] = C.x;
            buffer_array[j9+7] = C.y;
            buffer_array[j9+8] = C.z;
            j9+=9;
        }
    }
    return buffer ;
}

// Returns a Variant of 3 vec2's'for each each triangle dereferenced
// Used to get arrays for displaying with a simple shader
Variant GLTF::getFloatBuffer(std::vector<glm::vec2>& point_list, int material){
    int num_triangles = 0 ;
    for(int k=0;k<this->triangles.size();k++){
        if(this->triangles[k].material == material){
            num_triangles++;
        }
    }

    //TODO this access pattern prevents these Variant members from being private 
    // but using the constructor forces a copy that isn't necesarry 
    // Maybe here should be an array constructor that just takes type and size
    // and then the get array provides a shallow pointer that can't be freed?
    Variant buffer;
    buffer.type_ = Variant::FLOAT_ARRAY;
    buffer.ptr = (byte*)malloc(4 + num_triangles * 6 * sizeof(float));
    
    *((int*)buffer.ptr) = num_triangles * 6 ;// number of floats in array
    float* buffer_array =  (float*)(buffer.ptr+4) ; // pointer to start of float array
    int j6 = 0 ;
    for(int k=0;k<this->triangles.size();k++){
        if(this->triangles[k].material == material){
            Triangle& t = this->triangles[k];
            // A
            vec2& A = point_list[t.A];
            buffer_array[j6] = A.x;
            buffer_array[j6+1] = A.y;
            // B
            vec2& B = point_list[t.B];
            buffer_array[j6+2] = B.x;
            buffer_array[j6+3] = B.y;
            // C
            vec2& C = point_list[t.C];
            buffer_array[j6+4] = C.x;
            buffer_array[j6+5] = C.y;
            j6+=6;
        }
    }
    return buffer ;
}


// Returns a Variant of 3 vec4's'for each each triangle dereferenced
// Used to get arrays for displaying with a simple shader
Variant GLTF::getFloatBuffer(std::vector<glm::vec4>& point_list, int material){

    int num_triangles = 0 ;
    for(int k=0;k<this->triangles.size();k++){
        if(this->triangles[k].material == material){
            num_triangles++;
        }
    }

    //TODO this access pattern prevents these Variant members from being private 
    // but using the constructor forces a copy that isn't necesarry 
    // Maybe here should be an array constructor that just takes type and size
    // and then the get array provides a shallow pointer that can't be freed?
    Variant buffer;
    buffer.type_ = Variant::FLOAT_ARRAY;
    buffer.ptr = (byte*)malloc(4 + num_triangles * 12 * sizeof(float));
    
    *((int*)buffer.ptr) = num_triangles * 12 ;// number of floats in array
    float* buffer_array =  (float*)(buffer.ptr+4) ; // pointer to start of float array
    int j = 0 ;
    for(int k=0;k<this->triangles.size();k++){
        if(this->triangles[k].material == material){
            Triangle& t = this->triangles[k];
            // A
            vec4& A = point_list[t.A];
            buffer_array[j] = A.w;
            buffer_array[j+1] = A.x;
            buffer_array[j+2] = A.y;
            buffer_array[j+3] = A.z;
            // B
            vec4& B = point_list[t.B];
            buffer_array[j+4] = B.w;
            buffer_array[j+5] = B.x;
            buffer_array[j+6] = B.y;
            buffer_array[j+7] = B.z;
            // C
            vec4& C = point_list[t.C];
            buffer_array[j+8] = C.w;
            buffer_array[j+9] = C.x;
            buffer_array[j+10] = C.y;
            buffer_array[j+11] = C.z;
            j+=12;
        }
    }
    return buffer ;
}

// Returns a Variant of 3 vec4's'for each each triangle dereferenced
// Used to get arrays for displaying with a simple shader
Variant GLTF::getFloatBuffer(std::vector<glm::ivec4>& point_list, int material){

    int num_triangles = 0 ;
    for(int k=0;k<this->triangles.size();k++){
        if(this->triangles[k].material == material){
            num_triangles++;
        }
    }

    //TODO this access pattern prevents these Variant members from being private 
    // but using the constructor forces a copy that isn't necesarry 
    // Maybe here should be an array constructor that just takes type and size
    // and then the get array provides a shallow pointer that can't be freed?
    Variant buffer;
    buffer.type_ = Variant::FLOAT_ARRAY;
    buffer.ptr = (byte*)malloc(4 + num_triangles * 12 * sizeof(float));
    
    *((int*)buffer.ptr) = num_triangles * 12 ;// number of floats in array
    float* buffer_array =  (float*)(buffer.ptr+4) ; // pointer to start of float array
    int j = 0 ;
    for(int k=0;k<this->triangles.size();k++){
        if(this->triangles[k].material == material){
            Triangle& t = this->triangles[k];
            // A
            ivec4& A = point_list[t.A];
            buffer_array[j] = (float)A.w;
            buffer_array[j+1] = (float)A.x;
            buffer_array[j+2] = (float)A.y;
            buffer_array[j+3] = (float)A.z;
            // B
            ivec4& B = point_list[t.B];
            buffer_array[j+4] = (float)B.w;
            buffer_array[j+5] = (float)B.x;
            buffer_array[j+6] = (float)B.y;
            buffer_array[j+7] = (float)B.z;
            // C
            ivec4& C = point_list[t.C];
            buffer_array[j+8] = (float)C.w;
            buffer_array[j+9] = (float)C.x;
            buffer_array[j+10] = (float)C.y;
            buffer_array[j+11] = (float)C.z;
            j+=12;
        }
    }
    return buffer ;
}


// Returns a Variant of openGL triangle buffers for displaying this mesh in world space
Variant GLTF::getChangedBuffer(int selected_material){
    std::map<string, Variant> buffers;
    
    int num_triangles = 0 ;
    for(int k=0;k<this->triangles.size();k++){
        if(this->triangles[k].material == selected_material){
            num_triangles++;
        }
    }

    buffers["vertices"] = Variant(num_triangles*3);

    if(this->position_changed){
        Variant& pos_buffer = buffers["position"];
        pos_buffer.type_ = Variant::FLOAT_ARRAY;
        pos_buffer.ptr = (byte*)malloc(4 + num_triangles * 9 * sizeof(float));
        
        *((int*)pos_buffer.ptr) = num_triangles * 9 ;// number of floats in array
        float* pos_buffer_array =  (float*)(pos_buffer.ptr+4) ; // pointer to start of float array

        Variant& norm_buffer = buffers["normal"];
        norm_buffer.type_ = Variant::FLOAT_ARRAY;
        norm_buffer.ptr = (byte*)malloc(4 + num_triangles * 9 * sizeof(float));
        
        *((int*)norm_buffer.ptr) = num_triangles * 9 ;// number of floats in array
        float* norm_buffer_array =  (float*)(norm_buffer.ptr+4) ; // pointer to start of float array
        
        int j9 = 0 ;
        for(int k=0;k<this->triangles.size();k++){
            if(this->triangles[k].material == selected_material){
                Triangle& t = this->triangles[k];
                // A
                vec3& A = vertices[t.A].position;
                pos_buffer_array[j9] = A.x;
                pos_buffer_array[j9+1] = A.y;
                pos_buffer_array[j9+2] = A.z;
                // B
                vec3& B = vertices[t.B].position;
                pos_buffer_array[j9+3] = B.x;
                pos_buffer_array[j9+4] = B.y;
                pos_buffer_array[j9+5] = B.z;
                // C
                vec3& C = vertices[t.C].position;
                pos_buffer_array[j9+6] = C.x;
                pos_buffer_array[j9+7] = C.y;
                pos_buffer_array[j9+8] = C.z;

                
                // A
                vec3& A2 = vertices[t.A].normal;
                norm_buffer_array[j9] = A2.x;
                norm_buffer_array[j9+1] = A2.y;
                norm_buffer_array[j9+2] = A2.z;
                // B
                vec3& B2 = vertices[t.B].normal;
                norm_buffer_array[j9+3] = B2.x;
                norm_buffer_array[j9+4] = B2.y;
                norm_buffer_array[j9+5] = B2.z;
                // C
                vec3& C2 = vertices[t.C].normal;
                norm_buffer_array[j9+6] = C2.x;
                norm_buffer_array[j9+7] = C2.y;
                norm_buffer_array[j9+8] = C2.z;
                
                j9+=9;
            }
        }
    }    

    if(this->model_changed){
        vector<vec3> color ;
        vector<vec2> tex_coord;
        vector<vec4> weights;
        vector<ivec4> joints;
        for(const auto& v : this->vertices){
            color.push_back(v.color_mult);
            tex_coord.push_back(v.tex_coord);
            weights.push_back(v.weights);
            joints.push_back(v.joints);
        }

        buffers["color"] = this->getFloatBuffer(color, selected_material);
        buffers["tex_coord"] = this->getFloatBuffer(tex_coord, selected_material);    
        buffers["weights"] = this->getFloatBuffer(weights, selected_material);
        buffers["joints"] = this->getFloatBuffer(joints, selected_material);  

        const auto& mat = this->materials[selected_material] ;
        map<string,Variant> mat_map;
        mat_map["color"] = Variant(mat.color);
        mat_map["metallic"] = Variant(mat.roughness);
        mat_map["roughness"] = Variant(mat.roughness);
        mat_map["name"] = Variant(mat.name) ;
        mat_map["double_sided"] = Variant(mat.double_sided ? 1 : 0);
        mat_map["has_texture"] = Variant(mat.texture ? 1: 0);
        if(mat.texture){
            const Image& img = this->images[mat.image];
            mat_map["image_name"] = Variant(img.name);
            mat_map["image_width"] = Variant(img.width) ;
            mat_map["image_height"] = Variant(img.height) ;
            mat_map["image_channels"] = Variant(img.channels);
            mat_map["image_data"] = img.data.clone();
        }
        
        buffers["material"] = Variant(mat_map);


    }


    if(this->bones_changed){
        int num_bones = max_node_id+1 ;
        int shader_num_bones = 1024; // TODO avoid duplicate constant definition with bones texture
        Variant& bone_buffer = buffers["bones"];
        bone_buffer.type_ = Variant::FLOAT_ARRAY;
        bone_buffer.ptr = (byte*)malloc(4 + shader_num_bones * 16 * sizeof(float));
        *((int*)bone_buffer.ptr) = shader_num_bones * 16 ;// number of floats in array
        float* bone_buffer_array =  (float*)(bone_buffer.ptr+4) ; // pointer to start of float array

        for(const auto& [node_id, node] : nodes){
            if(node_id <= max_node_id){ // TODO I don't know how garbage gets in here, but it does on repeated loads sometimes
                memcpy(bone_buffer_array + (node_id*16), &(node.transform), 64);
            }
        }
    }



    return Variant(buffers);
}

void GLTF::setModel(const byte* data, int data_length){
    vector<Vertex> new_vertices;
    vector<Triangle> new_triangles;
    this->materials.clear();
    this->images.clear();
    this->animations.clear();
    joint_to_node = map<int,map<int,int>>();
    nodes = map<int,Node>();
    max_node_id = 0 ;
    root_nodes = vector<int>();

    //printf ("num bytes: %d \n", data_length);
    //printf("GLB Magic: %u  JSON_CHUNK: %u\n", 0x46546C67, 0x4E4F534A);
    uint magic_num = *(uint *)data;

    if(magic_num == 0x46546C67){ // check if GLTF
        uint version = *(int *)(data + 4);
        uint total_length = *(int *)(data + 8);
        printf("GLB file detected. Version : %u  Data Length: %u\n", version, total_length);
        //printf("File Length %d\n", data_length);

        uint JSON_length = *((uint *) (data + 12));// in bytes, not necessarily characters
        uint first_chunk_type = *((uint *) (data + 16));
        
        if(first_chunk_type == 0x4E4F534A){

            string header = string((char *) (data + 20), JSON_length);
            json = Variant::parseJSON(header);
            //json.printFormatted();
            int bin_chunk_start = 20 + JSON_length ;
            
            if(bin_chunk_start %4 != 0){
                bin_chunk_start += 4 - (bin_chunk_start %4);
            }
            uint bin_length = *((uint*)(data+bin_chunk_start)) ;
            uint second_chunk_type = *((uint*)(data+bin_chunk_start+4)) ;
            if(second_chunk_type == 0x004E4942){
                bin = Variant(data+bin_chunk_start+8, bin_length);

                int num_materials = json["materials"].getArrayLength();
                for(int k=0;k<num_materials;k++){
                    addMaterial(k, json, bin);
                }

                if(json["skins"].defined()){
                    vector<Variant> skins = json["skins"].getVariantArray();;
                    for(int s = 0; s < skins.size(); s++){
                        vector<Variant> skin_nodes = skins[s]["joints"].getVariantArray();
                        joint_to_node[s] = map<int,int>();
                        for(int k=0;k<skin_nodes.size();k++){
                            joint_to_node[s][k] = (int)(skin_nodes[k].getNumberAsFloat());
                        }
                    }
                }

                int default_scene = 0 ;
                if(json["scene"].defined()){
                    default_scene = json["scene"].getInt() ;
                }
                addScene(new_vertices, new_triangles, default_scene, json, bin) ;

                if(json["animations"].defined()){
                    vector<Variant> animations = json["animations"].getVariantArray();
                    for(Variant& animation : animations){
                        addAnimation(animation, json, bin);
                    }
                }
            }else{
                printf("Bin chunk not found after json !(got %d)\n", second_chunk_type);
            }
        }else{
            printf("first chunk not json: %u\n", first_chunk_type);
        }
    }else{
        printf("Not a GLB file! %d != %d\n", magic_num, 0x46546C67);
    }
    setModel(new_vertices, new_triangles);

}

GLTF::Accessor GLTF::access(int accessor_id, Variant& json, const Variant& bin){
    vector<Variant> accessors = json["accessors"].getVariantArray();
    vector<Variant> views = json["bufferViews"].getVariantArray() ;
    return access(accessor_id, accessors, views , bin);
   
}


GLTF::Accessor GLTF::access(int accessor_id, vector<Variant>& accessors, vector<Variant>& views, const Variant& bin){
     map<string,int> TYPE_LENGTH = {{"SCALAR",1},{"VEC2",2},{"VEC3",3},{"VEC4",4},{"MAT2",4},{"MAT3",9},{"MAT4",16}} ; // TODO static const
    map<int,int> COMPONENT_LENGTH = {{5120, 1},{5121, 1},{5122, 2},{5123, 2},{5125, 4},{5126, 4}}; // TODO static const

    
    auto& accessor = accessors[accessor_id];
    //printf("accessor:\n");
    //accessor.printFormatted();
    string type = accessor["type"].getString();
    uint c_type = (uint)(accessor["componentType"].getInt());
    int count = accessor["count"].getInt();
    int element_length = TYPE_LENGTH[type] * COMPONENT_LENGTH[c_type];

    auto& view = views[accessor["bufferView"].getInt()];
    int offset = 0 ;
    Variant v_offset = view["byteOffset"];
    if(v_offset.defined()){
        offset = v_offset.getInt();
    }
    Variant a_offset = accessor["byteOffset"] ;
    if(a_offset.defined()){
        offset += a_offset.getInt();
    }
    int byteLength = view["byteLength"].getInt();
    //printf("view:\n");
    //view.printFormatted();
    int stride = element_length;
    Variant v_stride = view["byteStride"] ;
    if(v_stride.defined()){
        stride = v_stride.getInt();
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
void GLTF::addPrimitive(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles,
                        Variant& primitive, int node_id, const glm::mat4& transform, Variant& json, const Variant& bin){
    //printf("Adding primitive:\n");
    //primitive.printFormatted();
    auto node = json["nodes"][node_id];
    int skin_id = node["skin"].defined() ? node["skin"].getInt() : 0;

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

    bool has_normals = false;
    GLTF::Accessor na ;
    float* normal_data = nullptr; // still held by Variant, not a memory leak
    if(primitive["attributes"]["NORMAL"].defined()){
        na = GLTF::access(primitive["attributes"]["NORMAL"].getInt(), json, bin);
        if(na.type == "VEC3" && na.component_type == 5126){
            has_normals = true;
            normal_data = na.data.getFloatArray();
            //printf("Got normals!\n");
        }else{
            printf("Normals are a weird type, skipping %s : %d \n" , na.type.c_str(), na.component_type);
        }
    }
    
    bool has_texcoords = false;
    GLTF::Accessor ta ;
    float* texcoords_data = nullptr; // still held by Variant, not a memory leak
    if(primitive["attributes"]["TEXCOORD_0"].defined()){
        ta = GLTF::access(primitive["attributes"]["TEXCOORD_0"].getInt(), json, bin);

        if(ta.type == "VEC2" && ta.component_type == 5126){
            has_texcoords = true;
            texcoords_data = ta.data.getFloatArray();
            //printf("Got Texture coordinates!\n");
        }else{
            printf("Texture coordinates are a weird type, skipping %s : %d \n" , ta.type.c_str(), ta.component_type);
        }
    }

    bool has_weights = false;
    GLTF::Accessor wa ;
    GLTF:Accessor ja;
    float* weight_data = nullptr; // still held by Variant, not a memory leak
    uint* joint_data = nullptr; // careful, could leak memory
    if(primitive["attributes"]["WEIGHTS_0"].defined()){
        wa = GLTF::access(primitive["attributes"]["WEIGHTS_0"].getInt(), json, bin);
        ja = GLTF::access(primitive["attributes"]["JOINTS_0"].getInt(), json, bin);
        if(wa.type == "VEC4" && wa.component_type == 5126){
            has_weights = true;
            weight_data = wa.data.getFloatArray();
            if(ja.component_type == 5123){
                //printf("unsigned short joints\n");
                int num_joints = ja.data.getArrayLength() ;
                short* shorts = ja.data.getShortArray();
                joint_data = (uint*)malloc(4*num_joints);
                for(int k=0;k<num_joints;k++){
                    joint_data[k] = (unsigned short)shorts[k];
                }
            }else if(ja.component_type == 5125){
                //printf("unsigned int joints\n");
                int num_joints = ja.data.getArrayLength() ;
                int* ints = ja.data.getIntArray();
                joint_data = (uint*)malloc(4*num_joints);
                for(int k=0;k<num_joints;k++){
                    joint_data[k] = (unsigned int)ints[k];
                }
            }else if(ja.component_type == 5121){
                //printf("unsigned byte joints\n");
                int num_joints = ja.data.getArrayLength() ;
                byte* bytes = ja.data.getByteArray();
                joint_data = (uint*)malloc(4*num_joints);
                for(int k=0;k<num_joints;k++){
                    joint_data[k] = bytes[k];
                }
            }else{
                printf("joint component type unsafe: %d \n " , ja.component_type);
            }
        }else{
            printf("Weights are a weird type, skipping %s : %d \n" , ta.type.c_str(), ta.component_type);
        }
    }


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
                printf("Indices are not a valid type ( %d)  aborting\n", ia.component_type);
                return ;
            }
        }else{
            printf("Indices are not scalar, aborting\n");
            return ;
        }
    }else{ // If no indices defined
        // indices are sequential
        num_indices = num_vertices;
        index_data = (uint*)malloc(4*num_indices);
        for(int k=0;k<num_indices;k++){
            index_data[k] = k ;
        }
    }

    int material = 0 ;
    if(primitive["material"].defined()){
        material = primitive["material"].getInt();
        //printf("Need material %d !\n", material);
    }else{
        printf("Failed to get material property!\n");
    }
    if(!json["materials"][material].defined()){
        printf("Failed to find material %d for primitive \n",  material);
    }
    
    vec3 mat_color = vec3(1,1,1);
    Variant iv = json["materials"][material]["pbrMetallicRoughness"]["baseColorTexture"]["index"] ;
    //int image = -1;
    if(iv.defined()){
        //printf("Found mesh primitive texture!\n");
        //image = iv.getInt();
        
        //printf("texture in primitive  %d x%d \n ", img.width, img.height);
    }else{
        Variant ic = json["materials"][material]["pbrMetallicRoughness"]["extensions"]["KHR_materials_pbrSpecularGlossiness"]["diffuseFactor"];
        if(!ic.defined()){
            ic = json["materials"][material]["pbrMetallicRoughness"]["baseColorFactor"];
        }
        if(ic.defined()){
            mat_color = vec3(ic[0].getNumberAsFloat(), ic[1].getNumberAsFloat(), ic[2].getNumberAsFloat());
        }
    }

    for(int k=0;k<num_vertices;k++){
        
        //printf("vertex: %f , %f , %f\n", point_data[3*k], point_data[3*k+1],point_data[3*k+2]);
        vec3 v_local = vec3(point_data[3*k], point_data[3*k+1],point_data[3*k+2]) ;
        vec4 v_global = transform*vec4(v_local,1);
        //printf("global: %f , %f , %f\n", v_global.x, v_global.y, v_global.z);
        Vertex v ;
        v.position = vec3(v_global);
        v.color_mult = mat_color;
        if(has_normals){
            vec3 n_local = vec3(normal_data[3*k], normal_data[3*k+1], normal_data[3*k+2]) ;
            vec4 n_global = transform*vec4(n_local,0);
            v.normal = vec3(n_global);
        }
        if(has_texcoords){
            v.tex_coord = vec2(texcoords_data[2*k], texcoords_data[2*k+1]) ;
        }
        if(has_weights){
            v.weights = vec4(weight_data[4*k], weight_data[4*k+1], weight_data[4*k+2], weight_data[4*k+3]) ;
            v.joints = ivec4(joint_to_node[skin_id][joint_data[4*k]],
                            joint_to_node[skin_id][joint_data[4*k+1]],
                            joint_to_node[skin_id][joint_data[4*k+2]],
                            joint_to_node[skin_id][joint_data[4*k+3]]) ;
            float scale = 1.0/(v.weights[0] + v.weights[1]+v.weights[2] + v.weights[3]);
            v.weights *= scale; ;
        }else{
            v.weights = vec4(1,0,0,0);
            v.joints = ivec4(node_id,0,0,0);
        }
        vertices.push_back(v);
    }

    for(int k=0;k<num_indices;k+=3){
        //printf("Triangle: %d , %d , %d\n", (int)index_data[k]+start_vertices, (int)index_data[k+1]+start_vertices,(int)index_data[k+2]+start_vertices);
        triangles.push_back({(int)index_data[k]+start_vertices,
                        (int)index_data[k+1]+start_vertices, 
                        (int)index_data[k+2]+start_vertices,
                        material});
    }


    free(index_data);
    if(joint_data != nullptr){
        free(joint_data);
    }
}

void GLTF::addMaterial(int material_id, Variant& json, const Variant& bin){
    if(this->materials.find(material_id) == this->materials.end()){
        //printf("Adding material %d!\n", material_id);
        Variant m_json = json["materials"][material_id];
        //m_json.printFormatted();
        Material mat ;
        if(m_json["name"].defined()){
            //printf("Got name!\n");
            mat.name = m_json["name"].getString();
        }
        // Doublesided can appear in two different places, prefer deeper one
        if(m_json["pbrMetallicRoughness"]["doubleSided"].defined()){
            //printf("Got double sided!\n");
            mat.double_sided = m_json["pbrMetallicRoughness"]["doubleSided"].getInt() > 0;
        }else if(m_json["doubleSided"].defined()){
            //printf("Got double sided!\n");
            mat.double_sided = m_json["doubleSided"].getInt() > 0;
        }
        if(m_json["pbrMetallicRoughness"]["metallicFactor"].defined()){
            //printf("Got metallic!\n");
            mat.metallic = m_json["pbrMetallicRoughness"]["metallicFactor"].getNumberAsFloat();
        }

        if(m_json["pbrMetallicRoughness"]["roughnessFactor"].defined()){
            //printf("Got roughness!\n");
            mat.roughness = m_json["pbrMetallicRoughness"]["roughnessFactor"].getNumberAsFloat();
        }

        if(m_json["pbrMetallicRoughness"]["baseColorFactor"].defined()){
            //printf("Got base color!\n");
            Variant c = m_json["pbrMetallicRoughness"]["baseColorFactor"] ;
            mat.color = vec3(c[0].getNumberAsFloat(), c[1].getNumberAsFloat(), c[2].getNumberAsFloat());
        }

        if(m_json["pbrMetallicRoughness"]["baseColorTexture"]["index"].defined()){
            //printf("Got base color texture index for %d!\n", material_id);
            mat.texture = true;
            mat.image = m_json["pbrMetallicRoughness"]["baseColorTexture"]["index"].getInt();
            addImage(mat.image, json, bin);
        }else if(m_json["pbrMetallicRoughness"]["extensions"]["KHR_materials_pbrSpecularGlossiness"]["diffuseTexture"]["index"].defined()){
            //printf("Got diffuse color texture index for %d!\n", material_id);
            mat.texture = true;
            mat.image = m_json["pbrMetallicRoughness"]["extensions"]["KHR_materials_pbrSpecularGlossiness"]["diffuseTexture"]["index"].getInt();
            addImage(mat.image, json, bin);
        }else{
            //printf("No texture index for %d!\n", material_id);
            mat.texture = false;
        }

        this->materials[material_id] = mat ;
        this->model_changed = true;
    }
}

void GLTF::addImage(int image_id, Variant& json, const Variant& bin){
    //printf("Adding image %d!\n", image_id);
    if(this->images.find(image_id) == this->images.end()){
        Image& img = this->images[image_id] ;
        Variant i_json = json["images"][image_id];
        if(!i_json.defined()){
            printf("Material referencing image not found!\n");
        }
        //i_json.printFormatted();
        if(i_json["name"].defined()){
            //printf("Got name!\n");
            img.name = i_json["name"].getString();
        }
        if(!json["bufferViews"][i_json["bufferView"]].defined()){
            printf("No buffer view on image, external resources not supported, aborting texture load.\n");
            return ;
        }

        auto view = json["bufferViews"][i_json["bufferView"]];
        int offset = 0 ;
        if(view["byteOffset"].defined()){
            offset = view["byteOffset"].getInt();
        }
        int byteLength = view["byteLength"].getInt();

        byte* pixels = stbi_load_from_memory(bin.ptr + 4 + offset, byteLength, &img.width, &img.height, &img.channels, 0) ;
        img.data = Variant(pixels,img.width*img.height*img.channels);
        free(pixels);
        printf("Loaded texture: %ix%ix%i = %d \n", img.width, img.height, img.channels, byteLength);
    }
}

void GLTF::addMesh(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles,
                   int mesh_id, int node_id, const glm::mat4& transform, Variant& json, const Variant& bin){
    //printf("Adding mesh %d!\n", mesh_id);

    auto primitives = json["meshes"][mesh_id]["primitives"].getVariantArray();
    for(int k=0;k<primitives.size();k++){
        addPrimitive(vertices, triangles, primitives[k], node_id, transform, json, bin);
    }
}

void GLTF::addNode(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles,
                   int node_id, const glm::mat4& transform, Variant& json, const Variant& bin){
    //printf("Adding node %d!\n", node_id);
    auto node = json["nodes"][node_id];
    Node& node_struct = nodes[node_id];
    max_node_id = std::max(max_node_id, node_id);
    if(node["name"].defined()){
        node_struct.name = node["name"].getString();
    }

    mat4 new_transform = transform ;
    if(node["matrix"].defined()){
        printf("Node has matrix not yet implemented!\n");
        //new_transform *= M;
    }else{
        Variant tv = node["translation"];
        if(tv.defined()){
            vec3 t = vec3(tv[0].getNumberAsFloat(), tv[1].getNumberAsFloat(), tv[2].getNumberAsFloat());
            node_struct.translation = t;
            node_struct.base_translation = t;
            new_transform = glm::translate(new_transform, t);
        }
        Variant rv = node["rotation"] ;
        if(rv.defined()){
            // GLB is XYZW but GLM:quat is WXYZ
            glm::quat qrot(rv[3].getNumberAsFloat(), rv[0].getNumberAsFloat(), rv[1].getNumberAsFloat(), rv[2].getNumberAsFloat() );
            node_struct.rotation = qrot;
            node_struct.base_rotation = qrot;
            new_transform *= glm::mat4_cast(qrot);
        }
        Variant sv = node["scale"];
        if(sv.defined()){
            vec3 s = vec3(sv[0].getNumberAsFloat(), sv[1].getNumberAsFloat(), sv[2].getNumberAsFloat());
            node_struct.scale = s;
            node_struct.base_scale = s;
            new_transform = glm::scale(new_transform, s);
        }
    }
    
    if(node["mesh"].defined()){
        int mesh_id = node["mesh"].getInt();
        
        addMesh(vertices, triangles, mesh_id, node_id, new_transform, json, bin);
    }
    if(node["children"].defined()){
        auto nodes = node["children"];
        for(int k=0;k<nodes.getArrayLength();k++){
            int child_id = nodes[k].getInt();
            node_struct.children.push_back(child_id);
            addNode(vertices, triangles, child_id, new_transform, json, bin);
        }
    }
}

void GLTF::addScene(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles,
                    int scene_id, Variant& json, const Variant& bin){
    printf("Adding scene %d!\n", scene_id);
    auto nodes = json["scenes"][scene_id]["nodes"];
    glm::mat4 ident(1);
    for(int k=0;k<nodes.getArrayLength();k++){
        int node_id = nodes[k].getInt();
        root_nodes.push_back(node_id);
        addNode(vertices, triangles, node_id, ident, json, bin);
    }
}


void GLTF::addAnimation(Variant& animation_json, Variant& json, const Variant& bin){
    //animation.printFormatted();
    
    Animation animation ;
    animation.name = animation_json["name"].getString();
    printf("Adding animation %s!\n", animation.name.c_str());

    vector<Variant> accessors = json["accessors"].getVariantArray();
    vector<Variant> views = json["bufferViews"].getVariantArray() ;

    vector<Variant> samplers_json = animation_json["samplers"].getVariantArray() ;
    vector<Variant> channels_json = animation_json["channels"].getVariantArray() ;
    for(Variant& channel_json : channels_json){
        AnimationChannel channel ;
        channel.node = (int)channel_json["target"]["node"].getNumberAsFloat();
        string type = channel_json["target"]["path"].getString();
        int vec_size = 0 ;
        if(type == "rotation"){
            channel.path = ROTATION;
            vec_size = 4;
        }else if(type == "scale"){
            channel.path = SCALE;
            vec_size = 3 ;
        }else if(type == "translation"){
            channel.path = TRANSLATION;
            vec_size = 3 ;
        }else{
            printf("Unrecognized animation channel path: %s . Aborting.\n", type.c_str());
            return;
        }

        Variant& sampler = samplers_json[channel_json["sampler"].getInt()];
        Accessor input = access(sampler["input"].getInt(), accessors, views, bin);
        Accessor output = access(sampler["output"].getInt(), accessors, views, bin);
        int num_samples = input.data.getArrayLength();
        float* times = input.data.getFloatArray();
        float* values = output.data.getFloatArray();
        for(int k=0;k<num_samples;k++){
            std::pair<float, vec4> sample;
            sample.first = times[k];
            if(vec_size == 4){
                sample.second = {values[k*4],values[k*4+1],values[k*4+2],values[k*4+3]};
                //sample.second = {values[k*4+1], values[k*4+2],values[k*4+3],values[k*4]};
                vec4& q = sample.second ;


            }else{
                sample.second = {values[k*3],values[k*3+1],values[k*3+2],0};
            }
            animation.duration = fmax(animation.duration,times[k]);
            channel.samples.push_back(sample);
        }
        animation.channels.push_back(channel);
    }

    this->animations.push_back(animation); // TODO id is ignored here, could be a bug source if we ever load them out of order
}


// Compacts the given vertices and sets the model to them
void GLTF::setModel(const std::vector<Vertex>& vertices, const std::vector<Triangle>& triangles){
    
    printf("set model internal called\n");
    this->vertices = vertices;
    this->triangles = triangles;

    vector<bool> unset_normal ;
    vector<bool> referenced ;
    for(int k=0; k<vertices.size(); k++){
        unset_normal.push_back(glm::length(vertices[k].normal) < 0.01);
        referenced.push_back(false);
    }

    // set undefined normals by summing up touching triangles
    for(int k=0; k < this->triangles.size(); k++){
        Triangle& t = this->triangles[k] ;
        vec3 n = this->getNormal(t);
        if(unset_normal[t.A]){
            this->vertices[t.A].normal += n ;
        }
        if(unset_normal[t.B]){
            this->vertices[t.B].normal += n ;
        }
        if(unset_normal[t.C]){
            this->vertices[t.C].normal += n ;
        }
        referenced[t.A] = true;
        referenced[t.B] = true;
        referenced[t.C] = true;
    }

    // removed vertices not in triangles
    vector<int> new_index ;
    int i = 0 ;
    vector<Vertex> new_vertices ;
    for(int k=0;k<this->vertices.size();k++){
        new_index.push_back(i);
        if(referenced[k]){
            this->vertices[k].normal = glm::normalize(this->vertices[k].normal) ; //normalize normals
            new_vertices.push_back(this->vertices[k]);
            i++;
        }  
    }


    for(int k=0; k < this->triangles.size(); k++){
        Triangle& t = this->triangles[k] ;
        t.A = new_index[t.A];
        t.B = new_index[t.B];
        t.C = new_index[t.C];
    }
    this->vertices = new_vertices ;
    
    //update AABB
    this->min = {9999999,9999999,9999999};
    this->max = {-9999999,-9999999,-9999999};
    for(int k=0;k<this->vertices.size(); k++){
        auto &v = this->vertices[k].position;
        if(v.x < this->min.x)this->min.x = v.x;
        if(v.y < this->min.y)this->min.y = v.y;
        if(v.z < this->min.z)this->min.z = v.z;
        if(v.x > this->max.x)this->max.x = v.x;
        if(v.y > this->max.y)this->max.y = v.y;
        if(v.z > this->max.z)this->max.z = v.z;
    }
    computeInvMatrices();
    this->model_changed = true;
    this->position_changed = true;
    this->bones_changed = true;

    printf("Total vertices: %d\n",(int) this->vertices.size());
    printf("Total triangles: %d\n",(int) this->triangles.size());
}

// hashes a vertex to allow duplicates to be detected and merged
int GLTF::hashVertex(vec3 v){
    return Variant(v).hash();
}

vec3 GLTF::getNormal(Triangle t){
    vec3 AB = this->vertices[t.B].position - this->vertices[t.A].position;
    vec3 AC = this->vertices[t.C].position - this->vertices[t.A].position;
    return glm::normalize(glm::cross(AB,AC));
}

// Given a ray in model space (p + v*t) return the t value of the nearest collision
// with the given triangle
// return negative if no collision
float EPSILON = 0.00001;
float GLTF::trace(Triangle tri, const vec3 &p, const vec3 &v){
    vector<Vertex>& x = this->vertices ;
    //TODO use the vector library
    vec3 AB = {x[tri.B].position.x-x[tri.A].position.x,x[tri.B].position.y-x[tri.A].position.y,x[tri.B].position.z-x[tri.A].position.z};
    vec3 AC = {x[tri.C].position.x-x[tri.A].position.x,x[tri.C].position.y-x[tri.A].position.y,x[tri.C].position.z-x[tri.A].position.z};
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
    vec3 s = {p.x-x[tri.A].position.x,p.y-x[tri.A].position.y,p.z-x[tri.A].position.z};
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
    last_traced_tri = -1 ;
    float min_t = std::numeric_limits<float>::max() ;
    for(int k=0;k<this->triangles.size();k++){
        float t = this->trace(this->triangles[k],p,v);
        if(t > 0 && t < min_t){
            min_t = t ;
            last_traced_tri  = k ;
        }
    }
    if(min_t < std::numeric_limits<float>::max() ){
        return min_t ;
    }return -1;
}

// Computes node transform matrices from their components and nesting
void GLTF::computeNodeMatrices(int node_id, const glm::mat4& transform){
    Node& node = nodes[node_id];
    node.transform = transform ;
    node.transform = glm::translate(node.transform, node.translation);
    node.transform *= glm::mat4_cast(node.rotation);
    node.transform = glm::scale(node.transform, node.scale);
    
    for(int k=0;k<node.children.size();k++){
        computeNodeMatrices(node.children[k], node.transform);
    }
    node.transform = node.transform * node.inv_transform;
}

void GLTF::computeNodeMatrices(){
    for(int k=0;k<root_nodes.size();k++){
        computeNodeMatrices(root_nodes[k], this->transform);
    } 
    this->bones_changed = true;
}

 // Computes base vertices for skinned vertices so they can later use apply node transforms
void GLTF::computeInvMatrices(){
    for(auto& [node_id, node] : nodes){
        if(node_id <= max_node_id){ // TODO I don't know how garbage gets in here, but it does on repeated loads sometimes
            node.inv_transform = mat4(1) ;
        }
    }
    for(int k=0;k<root_nodes.size();k++){
        computeNodeMatrices(root_nodes[k], glm::mat4(1.0f));
    } 
    for(auto& [node_id, node] : nodes){
        if(node_id <= max_node_id){ // TODO I don't know how garbage gets in here, but it does on repeated loads sometimes
            node.inv_transform = glm::inverse(node.transform) ;
        }
    }
}

// Applies current node transformed to skinned vertices
void GLTF::applyTransforms(){

    // need to flatten nodes into a vector since we're fetching them in the vertex loop
    //printf("Max node id: %d\n",max_node_id);
    vector<mat4> node_matrix(max_node_id+1) ;
    for(const auto& [node_id, node] : nodes){
        if(node_id <= max_node_id){ // TODO I don't know how garbage gets in here, but it does on repeated loads sometimes
            node_matrix[node_id] = node.transform ;
        }
    }
    
    for(auto& v : vertices){
        vec4 p = {0,0,0,0};
        vec4 n = {0,0,0,0};
        bool has_rigging = 0 ;
        const auto& joints = v.joints ;
        const auto& weights = v.weights ;
        for(int j=0;j<4;j++){
            if(weights[j] > 0){
                //printf("joint: %d\n", joints[j]);
                mat4& transform = node_matrix[joints[j]];
                p += transform * vec4(v.position,1) * weights[j] ;
                n += transform * vec4(v.normal,0) * weights[j];
                has_rigging = true;
            }
        }
        
        if(has_rigging){
            v.transformed_position = p;
            v.transformed_normal = n ;
        }else{ // no rigging, apply transform to starting position of vertices
            v.transformed_position = this->transform*vec4(v.position,1);
            v.transformed_normal = this->transform*vec4(v.normal,0);
        }
        v.transformed_normal = glm::normalize(v.transformed_normal);
    }
    
    this->position_changed = true;
}

void GLTF::setBasePose(){
    for(auto& [node_id, node] : nodes){
        if(node_id <= max_node_id){
            node.translation = node.base_translation;
            node.scale = node.base_scale;
            node.rotation = node.base_rotation;
        }
    }
}


// Sets transforms to the given enimation 
// Does not change transforms unaffected by snimation, does not apply transforms to vertices
void GLTF::animate(Animation& animation, float time){
    
    if(time < 0 || time > animation.duration){
        printf("Time outside of animation range, aborting!\n");
        return ;
    }

    for(int c=0;c<animation.channels.size();c++){
        AnimationChannel& channel = animation.channels[c];
        // Find the first frame after current time
        int end = channel.last_read ; // start from last read key for this channel
        if(channel.samples[end].first > time){ // if it's in the future
            end = 0 ; // start from the beginning
        }
        // walk forward to find the first frame after this time
        while(channel.samples[end].first < time && end < channel.samples.size()){\
            end++;
        }
        channel.last_read = end ;
        int start = end-1; // start frame is the frame before that
        
        float t = 0.5f;
        if(end ==channel.samples.size()){ // if time is beyond all keyframes
            end = start ; // start and end will both be the last frame
        }else if(end == 0){
            start = 0 ;
        }else{
            float start_time = channel.samples[start].first ;
            float end_time = channel.samples[end].first ;
            t = (time-start_time) / (end_time-start_time); 
        }

        if(channel.path == SCALE){
            nodes[channel.node].scale = channel.samples[start].second * (1-t) +  channel.samples[end].second * t;
        }else if(channel.path == TRANSLATION){
            nodes[channel.node].translation = channel.samples[start].second * (1-t) +  channel.samples[end].second * t;
        }else if(channel.path == ROTATION){
            auto start_quat = glm::quat(channel.samples[start].second[3], channel.samples[start].second[0], channel.samples[start].second[1], channel.samples[start].second[2]) ;
            auto end_quat = glm::quat(channel.samples[end].second[3], channel.samples[end].second[0], channel.samples[end].second[1], channel.samples[end].second[2]) ;
            if(t > 1 || t < 0){
                printf("invalid t: %f\n", t);
            }
            //nodes[channel.node].rotation = glm::mix(start_quat, end_quat, t);
            nodes[channel.node].rotation = GLTF::slerp(start_quat, end_quat, t);
        }
    }
    computeNodeMatrices();

    this->bones_changed = true;
}

glm::quat GLTF::slerp(glm::quat A, glm::quat B, float t){
    float cosTheta = glm::dot(A,B);
    if(cosTheta < 0){
        B*=-1;
    }
    if(cosTheta <.0001 || cosTheta > 0.9999){
        glm::quat result = glm::quat(
            A.w*(1-t) + B.w*t,
            A.x*(1-t) + B.x*t,
            A.y*(1-t) + B.y*t,
            A.z*(1-t) + B.z*t);
            result = glm::normalize(result);
        return result ;
    }else{
        float angle = acos(cosTheta);
        glm::quat result = (sin((1.0f - t) * angle) * A + sin(t * angle) * B) / sin(angle);
        result = glm::normalize(result);
        //printf("result: %f %f %f %f\n", result.w, result.x, result.y, result.z);
        return result ;
    }
}
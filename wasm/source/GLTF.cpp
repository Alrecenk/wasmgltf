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
using glm::dmat4;
using glm::dvec3;
typedef GLTF::Triangle Triangle;
using std::string;
using std::queue;


// Constructor
GLTF::GLTF(){
    this->buffers_changed = false;
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

// Returns a Variant of openGL triangle buffers for displaying this mesh_ in world_ space

Variant GLTF::getChangedBuffer(int selected_material){
    std::map<string, Variant> buffers;

    vector<vec3> position ;
    vector<vec3> normal ;
    vector<vec2> tex_coord;
    for(const auto& v : this->vertices){
        position.push_back(v.position);
        normal.push_back(v.normal);
        tex_coord.push_back(v.tex_coord);
    }
    

    buffers["position"] = this->getFloatBuffer(position, selected_material);
    buffers["normal"] = this->getFloatBuffer(normal, selected_material);
    buffers["tex_coord"] = this->getFloatBuffer(tex_coord, selected_material);
    

    vector<vec3> color ;
    for(const auto& v : this->vertices){
        color.push_back(v.color_mult);
    }

    buffers["color"] = this->getFloatBuffer(color, selected_material);
        
    
    buffers["vertices"] = Variant((int)(buffers["color"].getArrayLength()/3));

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

    return Variant(buffers);
}

void GLTF::setModel(const byte* data, int data_length){
    
    vector<Vertex> new_vertices;
    vector<Triangle> new_triangles;
    this->materials.clear();
    this->images.clear();

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

                int num_materials = json["materials"].getArrayLength();
                for(int k=0;k<num_materials;k++){
                    addMaterial(k, json, bin);
                }

                joint_to_node = map<int,map<int,int>>();
                nodes = map<int,Node>();
                root_nodes = vector<int>();

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
void GLTF::addPrimitive(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles,
                        const Variant& primitive, int skin_id, const glm::mat4& transform, const Variant& json, const Variant& bin){
    //printf("Adding primitive:\n");
    //primitive.printFormatted();
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
                printf("unsigned short joints\n");
                int num_joints = ja.data.getArrayLength() ;
                short* shorts = ja.data.getShortArray();
                joint_data = (uint*)malloc(4*num_joints);
                for(int k=0;k<num_joints;k++){
                    joint_data[k] = (unsigned short)shorts[k];
                }
            }else if(ja.component_type == 5125){
                printf("unsigned int joints\n");
                int num_joints = ja.data.getArrayLength() ;
                int* ints = ja.data.getIntArray();
                joint_data = (uint*)malloc(4*num_joints);
                for(int k=0;k<num_joints;k++){
                    joint_data[k] = (unsigned int)ints[k];
                }
            }else if(ja.component_type == 5121){
                printf("unsigned byte joints\n");
                int num_joints = ja.data.getArrayLength() ;
                byte* bytes = ja.data.getByteArray();
                joint_data = (uint*)malloc(4*num_joints);
                for(int k=0;k<num_joints;k++){
                    joint_data[k] = bytes[k];
                }
            }else{
                printf("joint comkponent type unsafe: %d \n " , ja.component_type);
            }
        }else{
            printf("Texture coordinates are a weird type, skipping %s : %d \n" , ta.type.c_str(), ta.component_type);
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

void GLTF::addMaterial(int material_id, const Variant& json, const Variant& bin){
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
        this->buffers_changed = true;
    }
}

void GLTF::addImage(int image_id, const Variant& json, const Variant& bin){
    //printf("Adding image %d!\n", image_id);
    if(this->images.find(image_id) == this->images.end()){
        Image& img = this->images[image_id] ;
        Variant i_json = json["images"][image_id];
        if(!i_json.defined()){
            printf("Material referencing image not found!\n");
        }
        //i_json.printFormatted();
        if(i_json["name"].defined()){
            printf("Got name!\n");
            img.name = i_json["name"].getString();
        }
        if(!json["bufferViews"][i_json["bufferView"]].defined()){
            printf("No buffer view on image, external resurcs not supported, aborting texture load.\n");
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
                   int mesh_id, int skin_id, const glm::mat4& transform, const Variant& json, const Variant& bin){
    //printf("Adding mesh %d!\n", mesh_id);

    auto primitives = json["meshes"][mesh_id]["primitives"];
    for(int k=0;k<primitives.getArrayLength();k++){
        addPrimitive(vertices, triangles, primitives[k], skin_id, transform, json, bin);
    }
}

void GLTF::addNode(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles,
                   int node_id, const glm::mat4& transform, const Variant& json, const Variant& bin){
    //printf("Adding node %d!\n", node_id);
    auto node = json["nodes"][node_id];
    Node& node_struct = nodes[node_id];
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
            new_transform = glm::translate(new_transform, t);
        }
        Variant rv = node["rotation"] ;
        if(rv.defined()){
            // GLB is XYZW but GLM:quat is WXYZ
            glm::quat qrot(rv[3].getNumberAsFloat(), rv[0].getNumberAsFloat(), rv[1].getNumberAsFloat(), rv[2].getNumberAsFloat() );
            node_struct.rotation = qrot;
            new_transform *= glm::mat4_cast(qrot);
        }
        Variant sv = node["scale"];
        if(sv.defined()){
            vec3 s = vec3(sv[0].getNumberAsFloat(), sv[1].getNumberAsFloat(), sv[2].getNumberAsFloat());
            node_struct.scale = s;
            new_transform = glm::scale(new_transform, s);
        }
    }
    
    if(node["mesh"].defined()){
        int mesh_id = node["mesh"].getInt();
        int skin_id = node["skin"].defined() ? node["skin"].getInt() : 0;
        addMesh(vertices, triangles, mesh_id, skin_id, new_transform, json, bin);
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
                    int scene_id, const Variant& json, const Variant& bin){
    printf("Adding scene %d!\n", scene_id);
    auto nodes = json["scenes"][scene_id]["nodes"];
    glm::mat4 ident(1);
    for(int k=0;k<nodes.getArrayLength();k++){
        int node_id = nodes[k].getInt();
        root_nodes.push_back(node_id);
        addNode(vertices, triangles, node_id, ident, json, bin);
    }
}


// Compacts the given vertices and sets the model to them
void GLTF::setModel(const std::vector<Vertex>& vertices, const std::vector<Triangle>& triangles){
    

    this->vertices = vertices;
    this->triangles = triangles;

    vector<bool> unset_normal ;
    for(int k=0; k<vertices.size(); k++){
        unset_normal.push_back(glm::length(vertices[k].normal) < 0.01);
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
    }
    //normalize normals
    for(int k=0;k<this->vertices.size();k++){
        this->vertices[k].normal = glm::normalize(this->vertices[k].normal) ;
    }
    
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
    computeBaseVertices();
    //nodes[74].rotation *= glm::quat(vec3(1.5,-0.2,-1.15));
    //applyNodeTransforms();
    this->buffers_changed = true;
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

// Changes all vertices within radius of origin to the given color
void GLTF::paint(const vec3 &center, const float &radius, const vec3 &color){
    float r2 = radius*radius;
    for(int k=0; k<this->vertices.size(); k++){
        vec3 d = {this->vertices[k].position.x-center.x,this->vertices[k].position.y-center.y,this->vertices[k].position.z-center.z};
        float d2 = d.x*d.x+d.y*d.y+d.z*d.z;
        if(d2<r2){
            // if actually changing
            if(this->vertices[k].color_mult.x != color.x || this->vertices[k].color_mult.y != color.y || this->vertices[k].color_mult.z != color.z ){
                this->vertices[k].color_mult = color;
                this->buffers_changed = true;
            }
        }
    }
}


// Computes absolute node matrices from their componentsand nesting
void GLTF::computeNodeMatrices(int node_id, const glm::mat4& transform){
    Node& node = nodes[node_id];
    node.absolute_transform = transform ;
    node.absolute_transform = glm::translate(node.absolute_transform, dvec3(node.translation));
    node.absolute_transform *= dmat4(glm::mat4_cast(node.rotation));
    node.absolute_transform = glm::scale(node.absolute_transform, dvec3(node.scale));
    for(int k=0;k<node.children.size();k++){
        computeNodeMatrices(node.children[k], node.absolute_transform);
    }
}

 // Computes base vertices for skinned vertices so they can later use apply node transforms
void GLTF::computeBaseVertices(){
    for(int k=0;k<root_nodes.size();k++){
        computeNodeMatrices(root_nodes[k], glm::mat4(1.0f));
    } 
    for(int k=0;k<vertices.size();k++){
        Vertex& v = vertices[k];
        v.base_position = vector<vec3>();
        v.base_normal = vector<vec3>();
        for(int j=0;j<4;j++){
            v.base_position.push_back({0,0,0});
            v.base_normal.push_back({0,0,0});
            if(v.weights[j] > 0){
                dmat4 transform = nodes[v.joints[j]].absolute_transform;
                dmat4 inv = glm::inverse(transform);
                v.base_position[j] = vec3(inv *  vec4(v.position, 1)) ;
                v.base_normal[j] = vec3(inv* vec4(v.normal, 0));
            }

        }
    }
}

// Applies current absolute node matrices to skinned vertices
void GLTF::applyNodeTransforms(){
    for(int k=0;k<root_nodes.size();k++){
        computeNodeMatrices(root_nodes[k], glm::mat4(1.0f));
    } 
    for(int k=0;k<vertices.size();k++){
        Vertex& v = vertices[k];
        v.position = {0,0,0};
        v.normal = {0,0,0};
        for(int j=0;j<4;j++){
            if(v.weights[j] > 0){
                dmat4 transform = nodes[v.joints[j]].absolute_transform ;
                v.position += vec3(transform * vec4(v.base_position[j], 1)) * v.weights[j];
                v.normal += vec3(transform * vec4(v.base_normal[j], 0)) * v.weights[j];
            }
        }
    }
}
#ifndef _GLTF_H_
#define _GLTF_H_ 1

#include "Variant.h"
#include <set>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/ext.hpp"

typedef Variant::Type Type;
typedef unsigned int uint;

class GLTF{

    public:

        struct Accessor{
            std::string type;
            uint component_type;
            Variant data;
        };

        struct Vertex{
            glm::vec3 position = {0, 0, 0}; // position in global space
            glm::vec3 normal = {0, 0, 0}; // normal in global space
            glm::vec2 tex_coord = {0, 0};
            glm::vec3 color_mult = {1.0f, 1.0f, 1.0f};

            glm::ivec4 joints = {0,0,0,0}; // Nodes this vertex is skinned to if any
            glm::vec4 weights = {0,0,0,0}; // weights for each skinning node
            
            glm::vec3 transformed_position ; // position in linear skin local space
            glm::vec3 transformed_normal ; //notmal in linjear skin local space
            
        };

        struct Triangle{
            int A;
            int B;
            int C;
            int material = -1;
        };

        struct Material{
            std::string name ="";
            bool double_sided = false;
            glm::vec3 color = {1.0f, 1.0f, 1.0f};
            float metallic = 1.0f;
            float roughness = 1.0f;
            bool texture = false; 
            int image ;
        };
            
        struct Image{
            std::string name = "";
            int width = 0 ;
            int height = 0 ;
            int channels = 0 ;
            Variant data; // byte array
        };

        struct Node{
            std::string name="" ;
            std::vector<int> children ;
            // Local transform is in components
            glm::quat rotation = {1.0f, 0.0f, 0.0f, 0.0f};
            glm::vec3 scale = {1.0f,1.0f,1.0f};
            glm::vec3 translation = {0.0f, 0.0f, 0.0f};

            glm::quat base_rotation = {1.0f, 0.0f, 0.0f, 0.0f};
            glm::vec3 base_scale = {1.0f,1.0f,1.0f};
            glm::vec3 base_translation = {0.0f, 0.0f, 0.0f};

            glm::mat4 inv_transform ; //transform from starting coordinates into bone space
            glm::mat4 transform ; // combines model position, inverse, and current node transform for vertex manipulation from base coordinates
        };

        enum Path {ROTATION, TRANSLATION, SCALE};

        struct AnimationChannel{
            int node =-1;
            Path path = SCALE;
            std::vector<std::pair<float,glm::vec4>> samples ;
            int last_read = 0 ;
        };

        struct Animation{
            std::string name = "";
            float duration = 0;
            std::vector<AnimationChannel> channels;
        };


        Variant json;
        Variant bin;
        std::map<int,std::map<int,int>> joint_to_node ; // joint_to_node[skin_id][joint_index] -> node_id
        std::map<int,Node> nodes ;
        int max_node_id;
        std::vector<int> root_nodes ;
        glm::mat4 transform;
        std::vector<Animation> animations;
        


        //int num_vertices;
        //int num_triangles;
        //std::vector<glm::vec3> vertices ; 
        //std::vector<glm::vec3> colors; 
       //std::vector<glm::vec3> normals;

        std::vector<Vertex> vertices ;
        std::vector<Triangle> triangles ; 
        std::map<int,Material> materials;
        std::map<int,Image> images;
        glm::vec3 min; // minimum values in each axis part of AABB
        glm::vec3 max; // maximum values in each axis part of AABB
        bool position_changed = false;
        bool model_changed = false;
        bool bones_changed = false;
        int last_traced_tri ; // Index of last triangle hit by raytrace

        // Constructor
        GLTF();

        //Destructor
        ~GLTF();
        
        
        Variant getChangedBuffer(int material);

        // Sets this Model to a chunk of raw GLB data
        void setModel(const byte* data, int data_length);

        // Compacts the given vertices and sets the model to them
        void setModel(const std::vector<Vertex>& vertices, const std::vector<Triangle>& triangles);

        void addPrimitive(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles,
            Variant& primitive, int skin_id, const glm::mat4& transform, Variant& json, const Variant& bin);

        void addMesh(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles,
            int mesh_id, int skin_id, const glm::mat4& transform, Variant& json, const Variant& bin);

        void addNode(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles,
            int node_id, const glm::mat4& transform, Variant& json, const Variant& bin);

        void addScene(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles,
            int scene_id, Variant& json, const Variant& bin);

        static Accessor access(int accessor_id, Variant& json, const Variant& bin);

        static Accessor access(int accessor_id, std::vector<Variant>& accessors, std::vector<Variant>& views, const Variant& bin);

        void addMaterial(int material_id, Variant& json, const Variant& bin);

        void addImage(int image_id, Variant& json, const Variant& bin);

        void addAnimation(Variant& animation, Variant& json, const Variant& bin);

        // Computes absolute node matrices from their componentsand nesting
        void computeNodeMatrices(int node_id, const glm::mat4& transform);

        // calls above with all roots usin the root transform
        void computeNodeMatrices();
        
        // Computes base vertices for skinned vertices so they can later use apply node transforms
        void computeInvMatrices();

        // Applies current absolute node matrices to skinned vertices
        void applyTransforms();

        void setBasePose();

        static glm::quat slerp(glm::quat A, glm::quat B, float t);

        // hashes a vertex to allow duplicates to be detected
        int hashVertex(glm::vec3 v);

        // returns the model with {vertices:float_array vertices, faces:(int_array or short_array) triangles}
        std::map<std::string,Variant> getModel();

        // Given a ray in model space (p + v*t) return the t value of the nearest collision
        // return negative if no collision
        float rayTrace(const glm::vec3 &p, const glm::vec3 &v);

        // Sets transforms to the given enimation 
        // Does not change transforms unaffected by snimation, does not apply transforms to vertices
        void animate(Animation& animation, float time);

    private:
        // Performs the duplicate work for the various get vertex buffer functions
        Variant getFloatBuffer(std::vector<glm::vec3>& ptr, int material);
        Variant getFloatBuffer(std::vector<glm::vec2>& ptr, int material);
        Variant getFloatBuffer(std::vector<glm::vec4>& ptr, int material);
        Variant getFloatBuffer(std::vector<glm::ivec4>& ptr, int material);

        // returns the normal of a triangle
        glm::vec3 getNormal(Triangle t);

        // Given a ray in model space (p + v*t) return the t value of the nearest collision
        // with the given triangle
        // return negative if no collision
        float trace(Triangle tri, const glm::vec3 &p, const glm::vec3 &v);

        
        

};
#endif // #ifndef _TRIANGLE_MESH_H_
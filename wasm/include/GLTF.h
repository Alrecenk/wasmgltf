#ifndef _GLTF_H_
#define _GLTF_H_ 1

#include "Variant.h"
#include <set>
#include "glm/glm.hpp"

typedef Variant::Type Type;
typedef unsigned int uint;

class GLTF{

    public:

        struct Accessor{
            std::string type;
            uint component_type;
            Variant data;
        };

        struct Triangle{
            int A;
            int B;
            int C;
        };

        Variant json;
        Variant bin;


        int num_vertices;
        int num_triangles;
        std::vector<glm::vec3> vertices ; 
        std::vector<glm::vec3> colors; 
        std::vector<glm::vec3> normals;
        std::vector<Triangle> triangles ; 
        glm::vec3 min; // minimum values in each axis part of AABB
        glm::vec3 max; // maximum values in each axis part of AABB
        bool vertices_changed = false;
        bool color_changed = false;

        // Constructor
        GLTF();

        //Destructor
        ~GLTF();
        
        // Returns a Variant of openGL triangle buffers for displaying this mesh_ in world_ space
        // result["position"] = float array of triangle vertices in order (Ax,Ay,Az,Bx,By,Bz,Cx,Cy,Cz)
        // result["normal] = same format for vertex normals
        // result["color"] = same format for colors but RGB floats from 0 to 1
        Variant getChangedBuffers();

        // Sets this Model to a chunk of raw GLB data
        void setModel(const byte* data, int data_length);

        // Compacts the given vertices and sets the model to them
        void setModel(const std::vector<glm::vec3>& vertices, const std::vector<Triangle>& triangles);

        static void addPrimitive(std::vector<glm::vec3>& vertices, std::vector<Triangle>& triangles,
            const Variant& primitive, const glm::mat4& transform, const Variant& json, const Variant& bin);

        static void addMesh(std::vector<glm::vec3>& vertices, std::vector<Triangle>& triangles,
            int mesh_id, const glm::mat4& transform, const Variant& json, const Variant& bin);

        static void addNode(std::vector<glm::vec3>& vertices, std::vector<Triangle>& triangles,
            int node_id, const glm::mat4& transform, const Variant& json, const Variant& bin);

        static void addScene(std::vector<glm::vec3>& vertices, std::vector<Triangle>& triangles,
            int scene_id, const Variant& json, const Variant& bin);

        static Accessor access(int accessor_id, const Variant& json, const Variant& bin);


        // hashes a vertex to allow duplicates ot be detected and merged
        int hashVertex(glm::vec3 v);

        // returns the model with {vertices:float_array vertices, faces:(int_array or short_array) triangles}
        std::map<std::string,Variant> getModel();

        // Given a ray in model space (p + v*t) return the t value of the nearest collision
        // return negative if no collision
        float rayTrace(const glm::vec3 &p, const glm::vec3 &v);

        // Changes all vertices within radius of origin to the given color
        void paint(const glm::vec3 &center, const float &radius, const glm::vec3 &color);

    private:
        // Performs the duplicate work for the various get vertex buffer functions
        Variant getFloatBuffer(std::vector<glm::vec3>& ptr);

        // returns the normal of a triangle
        glm::vec3 getNormal(Triangle t);

        // Given a ray in model space (p + v*t) return the t value of the nearest collision
        // with the given triangle
        // return negative if no collision
        float trace(Triangle tri, const glm::vec3 &p, const glm::vec3 &v);

};
#endif // #ifndef _TRIANGLE_MESH_H_
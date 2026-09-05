#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "GLHelpers.h"
// #include "Mesh.h"
#include <Eigen/Core>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

class ResourceManager {
	public:
		std::vector<VertexArrayObject*> _usedVAOs;
        std::vector<VertexBufferObject*> _usedVBOs;
        std::vector<ElementBufferObject*> _usedEBOs;
        std::vector<Texture*> _usedTextures;
        std::vector<Program*> _usedPrograms;

        ResourceManager();
        void addVAO(VertexArrayObject* _newVAO);
        void addVBO(VertexBufferObject* _newVBO);
        void addEBO(ElementBufferObject* _newEBO);
        void addTexture(Texture* _newTexture);
        void addProgram(Program* _newProgram, 
        			const std::string vertex_shader_filename,
					const std::string fragment_shader_filename,
					const std::string &fragment_data_name);
        void loadMeshes(int i);
        void loadTex(int i);
        void activeTexture(int i);
        void setFloat(int programid, std::string name, float f1);
        void setFloat(int programid, std::string name, float f1, float f2, float f3);
        void setInt(int programid, std::string name, int i1);
};

#endif
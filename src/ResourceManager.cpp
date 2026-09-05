#include "ResourceManager.h"

ResourceManager::ResourceManager() {

}

void ResourceManager::addVAO(VertexArrayObject* _newVAO) {
	_newVAO->init();
	_usedVAOs.push_back(_newVAO);
}

void ResourceManager::addVBO(VertexBufferObject* _newVBO) {
	_newVBO->init();
	_usedVBOs.push_back(_newVBO);
}

void ResourceManager::addEBO(ElementBufferObject* _newEBO) {
	_newEBO->init();
	_usedEBOs.push_back(_newEBO);
}

void ResourceManager::addTexture(Texture* _newTexture) {
	_newTexture->init();
	_usedTextures.push_back(_newTexture);
}

void ResourceManager::activeTexture(int i) {
	// _usedTextures[i]->activate();
	// _usedTextures[i]->bind();

	glActiveTexture(GL_TEXTURE0 + i); // + 0 -> normal; + 1 -> color: given fs using texture_color
    _usedTextures[i]->bind();

}

void ResourceManager::addProgram(Program* _newProgram,
					const std::string vertex_shader_filename,
					const std::string fragment_shader_filename,
					const std::string &fragment_data_name) {
	_newProgram->init(vertex_shader_filename, fragment_shader_filename, fragment_data_name);
	_usedPrograms.push_back(_newProgram);
}

void ResourceManager::loadMeshes(int i) {
	_usedVAOs[i]->bind();
    // _usedVBOs[i * 5]->bind();
    // _usedVBOs[i * 5 + 1]->bind();
    // _usedVBOs[i * 5 + 2]->bind();
    // _usedVBOs[i * 5 + 3]->bind();
    // _usedVBOs[i * 5 + 4]->bind();

    _usedEBOs[i]->bind();
    
    _usedPrograms[0]->bindVertexAttribArray("position", *(_usedVBOs[i * 5]), *(_usedEBOs[i]));
    _usedPrograms[0]->bindVertexAttribArray("vertex_normal",*(_usedVBOs[i* 5 + 1]), *(_usedEBOs[i]));
    _usedPrograms[0]->bindVertexAttribArray("tex_coord",*(_usedVBOs[i* 5 + 2]), *(_usedEBOs[i]));
    _usedPrograms[0]->bindVertexAttribArray("face_normal",*(_usedVBOs[i* 5 + 3]), *(_usedEBOs[i]));
    _usedPrograms[0]->bindVertexAttribArray("tangent",*(_usedVBOs[i* 5 + 4]), *(_usedEBOs[i]));

    // glBindVertexArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ResourceManager::loadTex(int i) {
	activeTexture(i * 4 + 0);
	activeTexture(i * 4 + 1);
	activeTexture(i * 4 + 2);
	activeTexture(i * 4 + 3);
	// activeTexture(i * 4 + 4);
	// printf("_usedTextures : %lu, i * 4 + 3: %d\n", _usedTextures.size(), i * 4 + 3);
	// if (i == 0) activeTexture(i * 4 + 4);
	// activeTexture(i * 5 + 4);

    glUniform1i(_usedPrograms[0]->uniform("tex_normal1"), i * 4 + 0); // d
    glUniform1i(_usedPrograms[0]->uniform("tex_normal2"), i * 4 + 1); // n
    glUniform1i(_usedPrograms[0]->uniform("tex_normal3"), i * 4 + 2); // g
    glUniform1i(_usedPrograms[0]->uniform("tex_normal4"), i * 4 + 3); // s
    // glUniform1i(_usedPrograms[0]->uniform("tex_normal5"), i * 5 + 5); // s
}

void ResourceManager::setFloat(int programid, std::string name, float f1) {
	_usedPrograms[programid]->bind();
	glUniform1f(_usedPrograms[programid]->uniform(name), f1);
}

void ResourceManager::setFloat(int programid, std::string name, float f1, float f2, float f3) {
	_usedPrograms[programid]->bind();
	glUniform3f(_usedPrograms[programid]->uniform(name), f1, f2, f3);
}

void ResourceManager::setInt(int programid, std::string name, int i1) {
	_usedPrograms[programid]->bind();
	glUniform1i(_usedPrograms[programid]->uniform(name), i1);
}
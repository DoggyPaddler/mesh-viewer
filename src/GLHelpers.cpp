#include "GLHelpers.h"
#include <SOIL/SOIL.h>

#include <iostream>
#include <fstream>

void Texture::init() {
	glGenTextures(1, &textureId1);
	// std::cout << "GL_TEXTURE0 " << GL_TEXTURE0 << std::endl;
	// std::cout << "textureId1: " << textureId1 << std::endl;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// std::cout << "init Texture id: " << textureId1 << std::endl;
	check_gl_error();
}

void Texture::bind() {
	glBindTexture(GL_TEXTURE_2D, textureId1);
	check_gl_error();
}

// void Texture::activate() {
// 	glActiveTexture(GL_TEXTURE0 + textureId1);
// 	check_gl_error();
// }

void Texture::update(const std::string &path_to_tex) {
	glBindTexture(GL_TEXTURE_2D, textureId1);
	unsigned char*imageData = NULL;
	int picWidth, picHeight, channels;
	//std::string fullName = path_to_tex + "diffuse.png";
	imageData = SOIL_load_image(path_to_tex.c_str(), &picWidth, &picHeight, &channels, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, picWidth, picHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(imageData);
	// glBindTexture(GL_TEXTURE_2D, 0);
	check_gl_error();
}

void VertexArrayObject::init() {
	glGenVertexArrays(1, &id);
	// std::cout << "init VAO id: " << id << std::endl;
	check_gl_error();
}

void VertexArrayObject::bind() {
	glBindVertexArray(id);
	// std::cout << "bind VAO id: " << id << std::endl;
	check_gl_error();
}

void VertexArrayObject::free() {
	glDeleteVertexArrays(1, &id);
	check_gl_error();
}

void VertexBufferObject::init() {
	glGenBuffers(1,&id);
	// std::cout << "init VBO id: " << id << std::endl;
	check_gl_error();
}

void VertexBufferObject::bind() {
	glBindBuffer(GL_ARRAY_BUFFER,id);
	// std::cout << "bind VBO id: " << id << std::endl;
	check_gl_error();
}

void VertexBufferObject::free() {
	glDeleteBuffers(1,&id);
	check_gl_error();
}

void VertexBufferObject::update(const Eigen::MatrixXf& M) {
	assert(id != 0);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*M.size(), M.data(), GL_STATIC_DRAW);
	rows = M.rows();
	cols = M.cols();
	check_gl_error();
}

void ElementBufferObject::init() {
	glGenBuffers(1,&id);
	// std::cout << "init EBO id: " << id << std::endl;
	check_gl_error();
}

void ElementBufferObject::bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,id);
	// std::cout << "bind EBO id: " << id << std::endl;
	check_gl_error();
}

void ElementBufferObject::free() {
	glDeleteBuffers(1,&id);
	check_gl_error();
}

void ElementBufferObject::update(const Eigen::MatrixXi& M) {
	assert(id != 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float)*M.size(), M.data(), GL_STATIC_DRAW);
	rows = M.rows();
	cols = M.cols();
	check_gl_error();
}

bool Program::init(
	const std::string vertex_shader_filename,
	const std::string fragment_shader_filename,
	const std::string tc_shader_filename,
	const std::string te_shader_filename,
	const std::string &fragment_data_name)
{
	tess = 1;
	using namespace std;
	vertex_shader = create_shader_helper(GL_VERTEX_SHADER, vertex_shader_filename);
	tc_shader = create_shader_helper(GL_TESS_CONTROL_SHADER, tc_shader_filename);
	te_shader = create_shader_helper(GL_TESS_EVALUATION_SHADER, te_shader_filename);
	fragment_shader = create_shader_helper(GL_FRAGMENT_SHADER, fragment_shader_filename);
	if (!vertex_shader || !fragment_shader || !tc_shader || !te_shader) {
		return false;
	}

	program_shader = glCreateProgram();
	if (0 == program_shader) {
		fprintf(stderr, "Error creating program object.");
		exit(1);
	}

	glAttachShader(program_shader, vertex_shader);
	glAttachShader(program_shader, tc_shader);
	glAttachShader(program_shader, te_shader);
	glAttachShader(program_shader, fragment_shader);
	glBindFragDataLocation(program_shader, 0, fragment_data_name.c_str());
	glLinkProgram(program_shader);
	GLint status;
	glGetProgramiv(program_shader, GL_LINK_STATUS, &status);

	if (status != GL_TRUE) {
		char buffer[512];
		glGetProgramInfoLog(program_shader, 512, NULL, buffer);
		cerr << "Linker error: " << endl << buffer << endl;
		program_shader = 0;
		return false;
	}

	check_gl_error();
	return true;
}


bool Program::init(
	const std::string vertex_shader_filename,
	const std::string fragment_shader_filename,
	const std::string &fragment_data_name)
{
	check_gl_error();
	tess = 0;
	using namespace std;
	check_gl_error();
	vertex_shader = create_shader_helper(GL_VERTEX_SHADER, vertex_shader_filename);
	fragment_shader = create_shader_helper(GL_FRAGMENT_SHADER, fragment_shader_filename);
	if (!vertex_shader || !fragment_shader) {
		return false;
	}

	program_shader = glCreateProgram();
	glAttachShader(program_shader, vertex_shader);
	glAttachShader(program_shader, fragment_shader);
	glBindFragDataLocation(program_shader, 0, fragment_data_name.c_str());
	glLinkProgram(program_shader);
	GLint status;
	glGetProgramiv(program_shader, GL_LINK_STATUS, &status);

	if (status != GL_TRUE) {
		char buffer[512];
		glGetProgramInfoLog(program_shader, 512, NULL, buffer);
		cerr << "Linker error: " << endl << buffer << endl;
		program_shader = 0;
		return false;
	}
	check_gl_error();
	return true;
}

void Program::bind() {
	glUseProgram(program_shader);
	check_gl_error();
}

GLint Program::attrib(const std::string &name) const {
	return glGetAttribLocation(program_shader, name.c_str());
}

GLint Program::uniform(const std::string &name) const {
	return glGetUniformLocation(program_shader, name.c_str());
}

GLint Program::bindVertexAttribArray(const std::string &name, VertexBufferObject& VBO, ElementBufferObject& EBO) const {
	GLint id = attrib(name);
	if (id < 0)
		return id;
	if (VBO.id == 0) {
		glDisableVertexAttribArray(id);
		return id;
	}
	VBO.bind();
	EBO.bind();
	glEnableVertexAttribArray(id);
	glVertexAttribPointer(id, VBO.rows, GL_FLOAT, GL_FALSE, 0, (void*)0);
	check_gl_error();

	return id;
}

void Program::free() {
	if (program_shader) {
		glDeleteProgram(program_shader);
		program_shader = 0;
	}
	if (vertex_shader) {
		glDeleteShader(vertex_shader);
		vertex_shader = 0;
	}
	if (fragment_shader) {
		glDeleteShader(fragment_shader);
		fragment_shader = 0;
	}
	check_gl_error();
}

std::string Program::read_glsl_file(const std::string &pathToFile) {
	std::string content;
	std::ifstream fileStream(pathToFile, std::ios::in);
	if(!fileStream.is_open()) {
		std::cerr << "Failed to read file from path: " << pathToFile << std::endl;
		return content;
	}
	std::string line = "";
	while(!fileStream.eof()) {
		std::getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}

GLuint Program::create_shader_helper(GLint type, const std::string &shader_filename){
	check_gl_error();
	using namespace std;
	std::string shader_string = read_glsl_file(shader_filename);
	if (shader_string.empty())
		return (GLuint) 0;
	check_gl_error();
	GLuint id = glCreateShader(type);
	const char *shader_string_const = shader_string.c_str();
	glShaderSource(id, 1, &shader_string_const, NULL);
	glCompileShader(id);
	check_gl_error();

	GLint status;
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE) {
		char buffer[512];
		if (type == GL_VERTEX_SHADER)
			cerr << "Vertex shader:" << endl;
		else if (type == GL_FRAGMENT_SHADER)
			cerr << "Fragment shader:" << endl;
		else if (type == GL_GEOMETRY_SHADER)
			cerr << "Geometry shader:" << endl;
		cerr << shader_string << endl << endl;
		glGetShaderInfoLog(id, 512, NULL, buffer);
		cerr << "Error: " << endl << buffer << endl;
		return (GLuint) 0;
	}
	check_gl_error();

	return id;
}

void _check_gl_error(const char *file, int line) {
	GLenum err (glGetError());

	while(err!=GL_NO_ERROR) {
	std::string error;

	switch(err)
	{
		case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
		case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
		case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
		case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
	}

	std::cerr << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
	err = glGetError();
	}
}

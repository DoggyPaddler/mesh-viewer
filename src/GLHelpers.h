#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>
#include <Eigen/Core>

#define GL_SILENCE_DEPRECATION
#ifdef _WIN32
#  include <windows.h>
#  undef max
#  undef min
#  undef DrawText
#endif

#ifndef __APPLE__
#  define GLEW_STATIC
#  include <GL/glew.h>
#endif

#ifdef __APPLE__
#   include <OpenGL/gl3.h>
#   define __gl_h_ 
# define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#else
#   ifdef _WIN32
#       include <windows.h>
#   endif
#   include <GL/gl.h>
#endif

class VertexArrayObject {
public:
	unsigned int id;
	VertexArrayObject() : id(0) {}
	void init();
	void bind();
	void free();
};

class VertexBufferObject {
public:
	typedef unsigned int GLuint;
	typedef int GLint;

	GLuint id;
	GLuint rows;
	GLuint cols;

	VertexBufferObject() : id(0), rows(0), cols(0) {}
	void init();
	void update(const Eigen::MatrixXf& M);
	void bind();
	void free();
};

class ElementBufferObject {
public:
	typedef unsigned int GLuint;
	typedef int GLint;

	GLuint id;
	GLuint rows;
	GLuint cols;

	ElementBufferObject() : id(0), rows(0), cols(0) {}
	void init();
	void update(const Eigen::MatrixXi& M);
	void bind();
	void free();
};

class Texture {
public:
	typedef unsigned int GLuint;
	typedef int GLint;

	GLuint textureId1;
	void init();
	void bind();
	void update(const std::string &path_to_tex);
	// void activate(int i);
	void free();
};

class Program {
public:
	typedef unsigned int GLuint;
	typedef int GLint;

	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint tc_shader;
	GLuint te_shader;
	GLuint program_shader;
	int tess;

	Program() : vertex_shader(0), fragment_shader(0), tc_shader(0), te_shader(0), program_shader(0) { }

	// Create a new shader from the specified source strings
	bool init(const std::string vertex_shader_filename,
	const std::string fragment_shader_filename,
	const std::string tc_shader_filename,
	const std::string te_shader_filename,
	const std::string &fragment_data_name);

	bool init(const std::string vertex_shader_filename,
	const std::string fragment_shader_filename,
	const std::string &fragment_data_name);

	// Select this shader for subsequent draw calls
	void bind();
	void free();
	GLint attrib(const std::string &name) const;
	GLint uniform(const std::string &name) const;
	GLint bindVertexAttribArray(const std::string &name, VertexBufferObject& VBO, ElementBufferObject& EBO) const;
	GLuint create_shader_helper(GLint type, const std::string &shader_filename);
	std::string read_glsl_file(const std::string &pathToFile);
};

// class 

void _check_gl_error(const char *file, int line);

#define check_gl_error() _check_gl_error(__FILE__,__LINE__)

#endif

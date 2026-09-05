#ifndef EDITOR_H
#define EDITOR_H

#include <fstream>
#include <string>
#include <iostream>

#include "GUI.h"
#include "Mesh.h"
// #include "Object.h"
#include "Camera.h"
#include "Cursor.h"
#include "ResourceManager.h"

using namespace std;
using namespace Eigen;

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

class Editor {
	public:
		int cur_mesh;
		int cur_tex;
		int render_mode;
		int useTex;
		Cursor* cursor;
		Camera* cam;
		ResourceManager *resources;
		std::vector<Mesh> mesh_vector;
		GLFWwindow* window;
		GUI gui;

		Editor(Eigen::Vector3f cam_init_pos, std::vector<std::string> obj_list, std::vector<std::string> tex_list); // Eigen::Vector3f 
		void insert_mesh(std::string filename, float scale);
		void add_tex(std::string foldername);
		void setGUICallbacks();

};

std::vector<std::string> directory_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes, bool save, bool multiple);

#endif
#ifndef EDITOR_H
#define EDITOR_H

#include <fstream>
#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <memory>

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
		std::atomic<bool> mesh_upload_in_progress;
		std::atomic<bool> mesh_upload_cancel_requested;
		std::atomic<float> mesh_upload_progress;
		std::atomic<int> mesh_upload_stage;
		std::mutex mesh_upload_mutex;
		std::unique_ptr<Mesh> pending_uploaded_mesh;
		std::thread mesh_upload_thread;
		std::string pending_uploaded_mesh_name;

		Editor(Eigen::Vector3f cam_init_pos, std::vector<std::string> obj_list, std::vector<std::string> tex_list); // Eigen::Vector3f 
		~Editor();
		void insert_mesh(std::string filename, float scale);
		void delete_mesh(int index);
		void delete_tex(int index);
		void start_mesh_upload(std::string filename, float scale);
		void cancel_mesh_upload();
		void poll_mesh_upload();
		void add_tex(std::string foldername);
		void setGUICallbacks();

};

std::vector<std::string> directory_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes, bool save, bool multiple);

#endif
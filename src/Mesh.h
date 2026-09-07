#ifndef MESH_H
#define MESH_H
#include <atomic>
#include <Eigen/Core>
#include <Eigen/Dense>
#include "Face.h"
// #include "read_off.h"
#include "ReadObj.h"

class Mesh {
	public:
		int start;
		// int num_face;
		int total_num_vertex;

		std::vector<Face> vec_F;
		std::vector<Eigen::Vector3f> vec_V;
		std::vector<Eigen::Vector3f> vec_N;
		std::vector<Eigen::Vector2f> vec_TC;

		Eigen::MatrixXf V;
		Eigen::MatrixXf N;
		Eigen::MatrixXf VN;
		Eigen::MatrixXf TC;

		Eigen::MatrixXf T;
		Eigen::MatrixXf B;

		Eigen::MatrixXi Indices;

		Eigen::Vector3f top_right;
		Eigen::Vector3f bottom_left;
		bool has_vt_mapping;
		bool load_cancelled;

		Mesh(std::string filename, float scale, std::atomic<float> *progress = nullptr, std::atomic<bool> *cancel_requested = nullptr, std::atomic<int> *stage = nullptr);

		void buildV(float scale);
		void buildVN();
		void buildTC();
		void buildBoxProjectionVT(std::atomic<float> *progress = nullptr, std::atomic<bool> *cancel_requested = nullptr);
		void buildN();

		void buildIndices();
};


#endif


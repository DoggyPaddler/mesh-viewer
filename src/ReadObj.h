#ifndef READOBJ_H
#define READOBJ_H

#include <vector>
#include <Eigen/Core>
#include "Face.h"


//   V  double matrix of vertex positions  #V by 3
//   TC  double matrix of texture coordinats #TC by 2
//   N  double matrix of corner normals #N by 3
//   F  #F list of face indices into vertex positions
//   FTC  #F list of face indices into vertex texture coordinates
//   FN  #F list of face indices into vertex normals
bool read_obj(
		const std::string obj_file_name,
		std::vector<Eigen::Vector3f> & V,
		std::vector<Eigen::Vector3f> & N,
		std::vector<Eigen::Vector2f> & TC,
		std::vector<std::vector<int>> & F,
		std::vector<std::vector<int>> & FTC,
		std::vector<std::vector<int>> & FN);

bool read_obj(
		const std::string obj_file_name,
		std::vector<Eigen::Vector3f> & V,
		std::vector<Eigen::Vector3f> & N,
		std::vector<Eigen::Vector2f> & TC,
		std::vector<Face> & F);

void append(std::vector<int> f_,
			int & cur_total_vertex,
			std::vector<Face> & F);
void append(std::vector<int> f_,
			std::vector<int> n_,
			int & cur_total_vertex,
			std::vector<Face> & F);
void append(std::vector<int> f_,
			std::vector<int> n_,
			std::vector<int> tc_,
			int & cur_total_vertex,
			std::vector<Face> & F);
void append_vtc(std::vector<int> f_,
			std::vector<int> n_,
			int & cur_total_vertex,
			std::vector<Face> & F);

#endif
#include "Object.h"
#include <iostream>

Object::Object(int init_type, Eigen::Vector3f init_pos, float init_scale) {
	type = init_type;
	model = Eigen::MatrixXf::Identity(4, 4);
	translation = Eigen::MatrixXf::Identity(4, 4);
	rotation = Eigen::MatrixXf::Identity(4, 4);
	scaling = Eigen::MatrixXf::Identity(4, 4);
	translation(0,3) = init_pos(0);
	translation(1,3) = init_pos(1);
	translation(2,3) = init_pos(2);

	Eigen::Matrix4f s = Eigen::MatrixXf::Identity(4, 4);
	s.topLeftCorner(3,3) << init_scale, 0, 0, 
							0, init_scale, 0,
							0, 0, init_scale;
	scaling = s;
	model = translation * rotation * scaling;
}
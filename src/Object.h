#ifndef OBJECT_H
#define OBJECT_H
#include <Eigen/Core>
#include <Eigen/Geometry>

class Object {
	public:
		int type;
		Eigen::Matrix4f model;
		Eigen::Matrix4f translation, rotation, scaling;

		Object(int init_type, Eigen::Vector3f init_pos, float init_scale);
};

#endif


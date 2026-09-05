#ifndef CAMERA_H
#define CAMERA_H

#include <Eigen/Core>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

class Camera {
	public:
		int pause;
		float alpha_tmp;
		float alpha_shift, beta_shift;
		float radius;									// distance from camera to origin

		Eigen::Matrix4f view;
		Eigen::Matrix4f camera;
		Eigen::Matrix4f persp;							// view & camera & projection transformation matrices
		Vector3f cam_pos;								// current camera position


		Camera(Eigen::Vector3f cam_init_pos); // Polar coordinate system
		void pause_or_resume(void);
		void update_camera(TimePoint t_now, TimePoint &t_start);
};

#endif
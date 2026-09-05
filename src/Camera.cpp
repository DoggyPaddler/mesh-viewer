#include "Camera.h"

Camera::Camera(Eigen::Vector3f cam_init_pos) {
	pause = 1;
	radius = cam_init_pos(0);
	alpha_shift = cam_init_pos(1);
	beta_shift = cam_init_pos(2);
	// alpha_tmp = 0;

	view = MatrixXf::Identity(4, 4);
	camera = MatrixXf::Identity(4, 4);

	float n = 2.0, f = 200.0, l = -1.0, r = 1.0, t = 1.0, b = -1.0;
	persp << 2*n/(r-l), 0, (r+l)/(r-l), 0,
			 0, 2*n/(t-b), (t+b)/(t-b), 0,
			 0, 0, (-1)*(f+n)/(f-n), (-1)*(2*f*n)/(f-n),
			 0, 0,  -1, 0;
}

void Camera::pause_or_resume(void) {
	pause = abs(pause-1);
	if (pause == 1) {
		alpha_shift = alpha_tmp;
	}
}

void Camera::update_camera(TimePoint t_now, TimePoint &t_start) {
	float alpha = 0.0, beta = 0.0;
    if (pause) {
        alpha = alpha_shift;
        t_start = std::chrono::high_resolution_clock::now();
    } else {
        alpha = alpha_shift + 0.2 * std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();;
    }
    beta = beta_shift;
    cam_pos = radius * Vector3f(cos(alpha)*cos(beta), sin(beta), sin(alpha)*cos(beta));

    Eigen::Matrix4f camera_d, camera_pos;
    camera_d << cos(M_PI/2.0-alpha),   0, -sin(M_PI/2.0-alpha), 0,
                -cos(alpha)*sin(beta), cos(beta),  -sin(alpha)*sin(beta),   0,
                cos(alpha)*cos(beta),  sin(beta),  sin(alpha)*cos(beta),    0,
                0,                     0, 0,                      1;
    camera_pos << 1, 0, 0, -cam_pos(0), 
			      0, 1, 0, -cam_pos(1), 
			      0, 0, 1, -cam_pos(2), 
			      0, 0, 0, 1;
    camera = camera_d * camera_pos;
    alpha_tmp = alpha;
}

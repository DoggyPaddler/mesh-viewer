#ifndef CURSOR_H
#define CURSOR_H

#include <Eigen/Core>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

class Cursor {
	public:
		Vector2f p0, p1;
		int right_key_pressed;
		int left_key_pressed;

		Cursor();
};

#endif
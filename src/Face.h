#ifndef FACE_H
#define FACE_H
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>

class Face {
	public:
		int start;
		int num_vertex;
		Eigen::VectorXi _vertex;
		Eigen::VectorXi _normal;
		Eigen::VectorXi _tex_coord;

		Face(int num_vertex, Eigen::VectorXi v);
		Face(int num_vertex, Eigen::VectorXi v, Eigen::VectorXi n);
		Face(int num_vertex, Eigen::VectorXi v, Eigen::VectorXi n, Eigen::VectorXi tc);
		Face(int num_vertex, Eigen::VectorXi v, Eigen::VectorXi tc, int _tc);
};

inline Face::Face(int num, Eigen::VectorXi v) {
	num_vertex = num;
	_vertex = v;
	start = 0;
}

inline Face::Face(int num, Eigen::VectorXi v, Eigen::VectorXi n) {
	num_vertex = num;
	_vertex = v;
	_normal = n;
	start = 0;
}

inline Face::Face(int num, Eigen::VectorXi v, Eigen::VectorXi n, Eigen::VectorXi tc) {
	num_vertex = num;
	_vertex = v;
	_normal = n;
	_tex_coord = tc;
	start = 0;
}

inline Face::Face(int num, Eigen::VectorXi v, Eigen::VectorXi tc, int _tc) {
	num_vertex = num;
	_vertex = v;
	_tex_coord = tc;
	start = 0;
}

Eigen::RowVector3f area_normal(const Eigen::RowVector3f v1, const Eigen::RowVector3f v2, const Eigen::RowVector3f v3);
void per_face_normals(const Eigen::MatrixXf & V, const Eigen::MatrixXi & F, Eigen::MatrixXf & N);
void per_vertex_normals(const Eigen::MatrixXf & V, const Eigen::MatrixXi & F, Eigen::MatrixXf & N);

void per_face_normals(const std::vector<Eigen::Vector3f> & V, const std::vector<Face> & F, Eigen::MatrixXf & N);
void per_vertex_normals(const std::vector<Eigen::Vector3f> & V, const std::vector<Face> & F, Eigen::MatrixXf & N);

inline Eigen::RowVector3f area_normal(const Eigen::RowVector3f v1, const Eigen::RowVector3f v2, const Eigen::RowVector3f v3) {
	Eigen::RowVector3f e1 = v1-v2;
	Eigen::RowVector3f e2 = v3-v2;
	Eigen::RowVector3f e3 = v3-v1;

	float cos = (e1.dot(e2))/(e1.norm()*e2.norm());
	float sin = sqrt(1-pow(cos,2));
	float area = (0.5)*e1.norm()*e2.norm()*sin;
	Eigen::RowVector3f normal = e1.cross(e2);
	normal = (-1) * area * normal.normalized();
	if (isnan(normal(0))) { normal(0) = 0.0; }
	if (isnan(normal(1))) { normal(1) = 0.0; }
	if (isnan(normal(2))) { normal(2) = 0.0; }
	return normal;
}

inline void per_face_normals(const Eigen::MatrixXf & V, const Eigen::MatrixXi & F, Eigen::MatrixXf & N) {
	N = Eigen::MatrixXf::Zero(F.rows(),3);
	for (int i=0; i<F.rows(); i++) {
		Eigen::RowVector3f v1 = V.row(F(i,0));
		Eigen::RowVector3f v2 = V.row(F(i,1));
		Eigen::RowVector3f v3 = V.row(F(i,2));
		Eigen::RowVector3f n = area_normal(v1,v2,v3);
		N.row(i) = n.normalized();
	}
}

inline void per_vertex_normals(const Eigen::MatrixXf & V, const Eigen::MatrixXi & F, Eigen::MatrixXf & N) {
	N = Eigen::MatrixXf::Zero(V.rows(),3);
	for (int i=0; i<V.rows(); i++) {
		Eigen::RowVector3f nv (0,0,0);
		for (int j=0; j<F.rows(); j++) {
			if ((F(j,0) == i) || (F(j,1) == i) || (F(j,2) == i)) {
				nv += area_normal(V.row(F(j,0)),V.row(F(j,1)),V.row(F(j,2)));
			}
		}
		N.row(i) = nv.normalized();
	}
}

inline void per_face_normals(const std::vector<Eigen::Vector3f> & V, const std::vector<Face> & F, Eigen::MatrixXf & N) {
	N = Eigen::MatrixXf::Zero(F.size(),3);
	for (int i=0; i<F.size(); i++) {
		Eigen::RowVector3f v1 = V[F[i]._vertex(0)];
		Eigen::RowVector3f v2 = V[F[i]._vertex(1)];
		Eigen::RowVector3f v3 = V[F[i]._vertex(2)];
		Eigen::RowVector3f n = area_normal(v1,v2,v3);
		N.row(i) = n.normalized();
	}
}

inline void per_vertex_normals(const std::vector<Eigen::Vector3f> & V, const std::vector<Face> & F, Eigen::MatrixXf & N) {
	N = Eigen::MatrixXf::Zero(V.size(),3);
	// A = Eigen::MatrixXf::Zero(V.size(),3);

	for (int k=0; k<F.size(); k++) {
		if (F[k].num_vertex == 3) {
			Eigen::RowVector3f area_n = area_normal(V[F[k]._vertex(0)],V[F[k]._vertex(1)],V[F[k]._vertex(2)]);
			N.row(F[k]._vertex(0)) = N.row(F[k]._vertex(0)) + area_n;
			N.row(F[k]._vertex(1)) = N.row(F[k]._vertex(1)) + area_n;
			N.row(F[k]._vertex(2)) = N.row(F[k]._vertex(2)) + area_n;

		} else if (F[k].num_vertex == 4) {
			Eigen::RowVector3f a = area_normal(V[F[k]._vertex(0)],V[F[k]._vertex(1)],V[F[k]._vertex(2)]);
			Eigen::RowVector3f b = area_normal(V[F[k]._vertex(0)],V[F[k]._vertex(2)],V[F[k]._vertex(3)]);
			N.row(F[k]._vertex(0)) += (a + b);
			N.row(F[k]._vertex(1)) += (a + b);
			N.row(F[k]._vertex(2)) += (a + b);
			N.row(F[k]._vertex(3)) += (a + b);
		}
	}
	for (int i=0; i<V.size(); i++) {
		N.row(i) = N.row(i).normalized();
	}
}
#endif


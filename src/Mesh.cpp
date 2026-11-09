#include "Mesh.h"
#include <iostream>
#include <map>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace {

struct AOTriangle {
	Eigen::Vector3f a;
	Eigen::Vector3f b;
	Eigen::Vector3f c;
	Eigen::Vector3f bounds_min;
	Eigen::Vector3f bounds_max;
	Eigen::Vector3f centroid;
};

struct AOBvhNode {
	Eigen::Vector3f bounds_min;
	Eigen::Vector3f bounds_max;
	int start = 0;
	int count = 0;
	int left = -1;
	int right = -1;
};

class AOTriangleBvh {
public:
	explicit AOTriangleBvh(const Eigen::MatrixXf &vertices, const Eigen::MatrixXi &indices) {
		triangles.reserve(indices.rows());
		for (int i = 0; i < indices.rows(); ++i) {
			AOTriangle triangle;
			triangle.a = vertices.row(indices(i, 0));
			triangle.b = vertices.row(indices(i, 1));
			triangle.c = vertices.row(indices(i, 2));
			triangle.bounds_min = triangle.a.cwiseMin(triangle.b).cwiseMin(triangle.c);
			triangle.bounds_max = triangle.a.cwiseMax(triangle.b).cwiseMax(triangle.c);
			triangle.centroid = (triangle.a + triangle.b + triangle.c) / 3.0f;
			if ((triangle.b - triangle.a).cross(triangle.c - triangle.a).squaredNorm() > 1e-16f) {
				triangles.push_back(triangle);
			}
		}

		order.resize(triangles.size());
		std::iota(order.begin(), order.end(), 0);
		if (!order.empty()) {
			buildNode(0, (int)order.size());
		}
	}

	bool occluded(const Eigen::Vector3f &origin, const Eigen::Vector3f &direction,
			   float max_distance, float ray_epsilon) const {
		return !nodes.empty() && intersectsNode(0, origin, direction, max_distance, ray_epsilon);
	}

private:
	std::vector<AOTriangle> triangles;
	std::vector<int> order;
	std::vector<AOBvhNode> nodes;

	int buildNode(int start, int end) {
		const int node_index = (int)nodes.size();
		nodes.emplace_back();
		AOBvhNode &node = nodes[node_index];
		node.bounds_min = Eigen::Vector3f::Constant(std::numeric_limits<float>::max());
		node.bounds_max = Eigen::Vector3f::Constant(std::numeric_limits<float>::lowest());
		Eigen::Vector3f centroid_min = node.bounds_min;
		Eigen::Vector3f centroid_max = node.bounds_max;
		for (int i = start; i < end; ++i) {
			const AOTriangle &triangle = triangles[order[i]];
			node.bounds_min = node.bounds_min.cwiseMin(triangle.bounds_min);
			node.bounds_max = node.bounds_max.cwiseMax(triangle.bounds_max);
			centroid_min = centroid_min.cwiseMin(triangle.centroid);
			centroid_max = centroid_max.cwiseMax(triangle.centroid);
		}

		const int triangle_count = end - start;
		if (triangle_count <= 8) {
			node.start = start;
			node.count = triangle_count;
			return node_index;
		}

		Eigen::Vector3f extent = centroid_max - centroid_min;
		int axis = 0;
		if (extent.y() > extent.x()) axis = 1;
		if (extent.z() > extent(axis)) axis = 2;
		const int middle = start + triangle_count / 2;
		std::nth_element(order.begin() + start, order.begin() + middle, order.begin() + end,
			[this, axis](int lhs, int rhs) {
				return triangles[lhs].centroid(axis) < triangles[rhs].centroid(axis);
			});
		const int left = buildNode(start, middle);
		const int right = buildNode(middle, end);
		nodes[node_index].left = left;
		nodes[node_index].right = right;
		return node_index;
	}

	static bool intersectsBounds(const AOBvhNode &node, const Eigen::Vector3f &origin,
							  const Eigen::Vector3f &direction, float max_distance) {
		float near_distance = 0.0f;
		float far_distance = max_distance;
		for (int axis = 0; axis < 3; ++axis) {
			if (std::fabs(direction(axis)) < 1e-8f) {
				if (origin(axis) < node.bounds_min(axis) || origin(axis) > node.bounds_max(axis)) {
					return false;
				}
				continue;
			}
			const float inverse_direction = 1.0f / direction(axis);
			float first = (node.bounds_min(axis) - origin(axis)) * inverse_direction;
			float second = (node.bounds_max(axis) - origin(axis)) * inverse_direction;
			if (first > second) std::swap(first, second);
			near_distance = std::max(near_distance, first);
			far_distance = std::min(far_distance, second);
			if (far_distance < near_distance) return false;
		}
		return far_distance > 0.0f;
	}

	static bool intersectsTriangle(const AOTriangle &triangle, const Eigen::Vector3f &origin,
							   const Eigen::Vector3f &direction, float max_distance, float ray_epsilon) {
		const Eigen::Vector3f edge1 = triangle.b - triangle.a;
		const Eigen::Vector3f edge2 = triangle.c - triangle.a;
		const Eigen::Vector3f p = direction.cross(edge2);
		const float determinant = edge1.dot(p);
		if (std::fabs(determinant) < 1e-8f) return false;

		const float inverse_determinant = 1.0f / determinant;
		const Eigen::Vector3f t = origin - triangle.a;
		const float u = t.dot(p) * inverse_determinant;
		if (u < 0.0f || u > 1.0f) return false;
		const Eigen::Vector3f q = t.cross(edge1);
		const float v = direction.dot(q) * inverse_determinant;
		if (v < 0.0f || u + v > 1.0f) return false;
		const float distance = edge2.dot(q) * inverse_determinant;
		return distance > ray_epsilon && distance < max_distance;
	}

	bool intersectsNode(int node_index, const Eigen::Vector3f &origin,
					 const Eigen::Vector3f &direction, float max_distance, float ray_epsilon) const {
		const AOBvhNode &node = nodes[node_index];
		if (!intersectsBounds(node, origin, direction, max_distance)) return false;
		if (node.count > 0) {
			for (int i = node.start; i < node.start + node.count; ++i) {
				if (intersectsTriangle(triangles[order[i]], origin, direction, max_distance, ray_epsilon)) {
					return true;
				}
			}
			return false;
		}
		return intersectsNode(node.left, origin, direction, max_distance, ray_epsilon) ||
			intersectsNode(node.right, origin, direction, max_distance, ray_epsilon);
	}
};

}

void Mesh::buildBoxProjectionVT(std::atomic<float> *progress, std::atomic<bool> *cancel_requested) {
	if (vec_V.empty()) {
		vec_TC.clear();
		return;
	}

	Eigen::Vector3f min_v = vec_V[0];
	Eigen::Vector3f max_v = vec_V[0];
	for (int i = 1; i < (int)vec_V.size(); ++i) {
		if (cancel_requested != nullptr && cancel_requested->load()) {
			return;
		}
		min_v = min_v.cwiseMin(vec_V[i]);
		max_v = max_v.cwiseMax(vec_V[i]);
	}

	const float eps = 1e-6f;
	float range_x = std::max(max_v(0) - min_v(0), eps);
	float range_y = std::max(max_v(1) - min_v(1), eps);
	float range_z = std::max(max_v(2) - min_v(2), eps);

	Eigen::MatrixXf normals;
	per_vertex_normals(vec_V, vec_F, normals);

	vec_TC.clear();
	vec_TC.resize(vec_V.size(), Eigen::Vector2f::Zero());

	for (int i = 0; i < (int)vec_V.size(); ++i) {
		if (cancel_requested != nullptr && cancel_requested->load()) {
			return;
		}

		Eigen::Vector3f abs_n = normals.row(i).cwiseAbs();
		const Eigen::Vector3f &p = vec_V[i];

		float u = 0.0f;
		float v = 0.0f;
		if (abs_n(0) >= abs_n(1) && abs_n(0) >= abs_n(2)) {
			u = (p(2) - min_v(2)) / range_z;
			v = (p(1) - min_v(1)) / range_y;
		} else if (abs_n(1) >= abs_n(0) && abs_n(1) >= abs_n(2)) {
			u = (p(0) - min_v(0)) / range_x;
			v = (p(2) - min_v(2)) / range_z;
		} else {
			u = (p(0) - min_v(0)) / range_x;
			v = (p(1) - min_v(1)) / range_y;
		}

		vec_TC[i] = Eigen::Vector2f(u, v);

		if (progress != nullptr && (i % 2048 == 0)) {
			float t = (float)i / std::max(1, (int)vec_V.size());
			progress->store(0.70f + 0.12f * t);
		}
	}

	for (int i = 0; i < (int)vec_F.size(); ++i) {
		if (cancel_requested != nullptr && cancel_requested->load()) {
			return;
		}
		vec_F[i]._tex_coord = Eigen::VectorXi::Zero(vec_F[i].num_vertex);
		for (int j = 0; j < vec_F[i].num_vertex; ++j) {
			vec_F[i]._tex_coord(j) = vec_F[i]._vertex(j);
		}
	}

	if (progress != nullptr) {
		progress->store(0.82f);
	}
}

void Mesh::buildVN() {
	int cur_num_vertex = 0;
	VN.conservativeResize(vec_V.size(), 3);
	if (vec_N.size() == 0) {
		Eigen::MatrixXf mesh_vn;
		per_vertex_normals(vec_V, vec_F, mesh_vn);
		cur_num_vertex = 0;
		for (int i=0; i<vec_F.size(); i++) {
			for (int j=0; j<vec_F[i].num_vertex; j++) {
				int rv = vec_F[i]._vertex(j);
				VN.row(rv) = mesh_vn.row(rv);
			}
		}

	} else {
		for (int i=0; i<vec_F.size(); i++) {
			for (int j=0; j<vec_F[i].num_vertex; j++) {
				int rn = vec_F[i]._normal(j);
				int rv = vec_F[i]._vertex(j);
				VN.row(rv) = vec_N[rn];
				// printf("rn: %d, rv: %d\n", rn, rv);
				// std::cout << VN.row(rv) << std::endl;
			}
		}
	}
}

void Mesh::buildTC() {
	int cur_num_vertex = 0;
	float maxW = 0.0;
	float maxH = 0.0;
	TC.conservativeResize(vec_V.size(), 2);
	if (vec_TC.size() != 0) {
		for (int i=0; i<vec_F.size(); i++) {
			for (int j=0; j<vec_F[i].num_vertex; j++) {
				int r = vec_F[i]._tex_coord(j);
				int rv = vec_F[i]._vertex(j);
				TC.row(rv) = vec_TC[r];
				if (vec_TC[r](0) > maxH) {
					maxH = vec_TC[r](0);
				}
				if (vec_TC[r](1) > maxW) {
					maxW = vec_TC[r](1);
				}
			}
		}
		TC = TC / std::max(maxW, maxH);
	} else {
		TC = Eigen::MatrixXf::Zero(vec_V.size(), 2);
	}

	cur_num_vertex = 0;
	T.conservativeResize(vec_V.size(), 3);
	for (int i=0; i<vec_F.size(); i++) {
		int rv0 = vec_F[i]._vertex(0);
		int rv1 = vec_F[i]._vertex(1);
		int rv2 = vec_F[i]._vertex(2);

		Eigen::Vector3f pos1 = V.row(rv0);
		Eigen::Vector3f pos2 = V.row(rv1);
		Eigen::Vector3f pos3 = V.row(rv2);

		Eigen::Vector2f uv1 = TC.row(rv0);
		Eigen::Vector2f uv2 = TC.row(rv1);
		Eigen::Vector2f uv3 = TC.row(rv2);

		Eigen::Vector3f edge1 = pos2 - pos1;
		Eigen::Vector3f edge2 = pos3 - pos1;
		Eigen::Vector2f deltaUV1 = uv2 - uv1;
		Eigen::Vector2f deltaUV2 = uv3 - uv1;

		float denom = deltaUV1(0) * deltaUV2(1) - deltaUV1(1) * deltaUV2(0);
		Eigen::Vector3f tangt(1.0f, 0.0f, 0.0f);
		if (std::fabs(denom) > 1e-8f) {
			float f = 1.0f / denom;
			tangt = (edge1 * deltaUV2(1) - edge2 * deltaUV1(1)) * f;
		}
		tangt.normalize();

		for (int j=0; j<vec_F[i].num_vertex; j++) {
			int rv = vec_F[i]._vertex(j);
			T.row(rv) = tangt;
		}
	}
}

void Mesh::buildN() {
	Eigen::MatrixXf new_fn;
	per_face_normals(vec_V, vec_F, new_fn);

	N.conservativeResize(vec_V.size(), 3);
	for (int i=0; i < vec_F.size(); i++){
		for (int j=0; j<vec_F[i].num_vertex; j++) {
			int rv = vec_F[i]._vertex(j);
			N.row(rv) = new_fn.row(i);
		}
	}
}

void Mesh::buildV(float scale){
	float total_area = 0.0;
	Eigen::Vector3f mesh_bc (0.0,0.0,0.0);
	V.conservativeResize(vec_V.size(), 3);
	for (int i=0; i<vec_V.size(); i++) {
		V.row(i) = scale* vec_V[i];
	}

	for (int i=0; i<vec_F.size(); i++) {
		//int r = vec_F[i]._vertex(j);
		Eigen::Vector3f v1 = scale* vec_V[vec_F[i]._vertex(0)];
		Eigen::Vector3f v2 = scale* vec_V[vec_F[i]._vertex(1)];
		Eigen::Vector3f v3 = scale* vec_V[vec_F[i]._vertex(2)];

		Eigen::Vector3f bc ((v1(0)+v2(0)+v3(0))/3, (v1(1)+v2(1)+v3(1))/3, (v1(2)+v2(2)+v3(2))/3);
		float area = 0.5 * ((v2-v1).cross(v3-v1)).norm();
		mesh_bc += (area * bc);
		total_area += area;
	}

	mesh_bc = mesh_bc/total_area;
	for (int i = 0; i < vec_V.size(); i++) {
		V.row(i) << V(i,0) - mesh_bc(0), V(i,1) - mesh_bc(1), V(i,2) - mesh_bc(2); 
	}
}

void Mesh::buildIndices() {
	Indices.conservativeResize(vec_F.size(), 3);
	for (int i=0; i<vec_F.size(); i++) {
		// Indices.row(i*3 + 0) << vec_F[i]._vertex(0);
		// Indices.row(i*3 + 1) << vec_F[i]._vertex(1);
		// Indices.row(i*3 + 2) << vec_F[i]._vertex(2);
		Indices.row(i) << vec_F[i]._vertex(0), vec_F[i]._vertex(1), vec_F[i]._vertex(2);
	}
}

void Mesh::buildAO(std::atomic<float> *progress, std::atomic<bool> *cancel_requested) {
	const int vertex_count = (int)V.rows();
	AO = Eigen::MatrixXf::Ones(vertex_count, 1);
	if (vertex_count == 0 || Indices.rows() == 0 || VN.rows() != vertex_count) {
		return;
	}

	AOTriangleBvh bvh(V, Indices);
	const Eigen::Vector3f bounds_min = V.colwise().minCoeff().transpose();
	const Eigen::Vector3f bounds_max = V.colwise().maxCoeff().transpose();
	const float mesh_extent = (bounds_max - bounds_min).norm();
	const float ray_bias = std::max(mesh_extent * 1e-4f, 1e-6f);
	const float ray_epsilon = ray_bias * 0.25f;
	const float max_distance = std::max(mesh_extent * 0.35f, ray_bias * 4.0f);
	const int sample_count = 32;
	const float golden_angle = 2.39996323f;

	for (int vertex = 0; vertex < vertex_count; ++vertex) {
		if (cancel_requested != nullptr && cancel_requested->load()) {
			return;
		}

		Eigen::Vector3f normal = VN.row(vertex).transpose();
		if (normal.squaredNorm() < 1e-12f) {
			AO(vertex, 0) = 1.0f;
			continue;
		}
		normal.normalize();
		const Eigen::Vector3f reference = std::fabs(normal.y()) < 0.999f
			? Eigen::Vector3f::UnitY()
			: Eigen::Vector3f::UnitX();
		const Eigen::Vector3f tangent = reference.cross(normal).normalized();
		const Eigen::Vector3f bitangent = normal.cross(tangent);
		const Eigen::Vector3f origin = V.row(vertex).transpose() + normal * ray_bias;
		int occluded_rays = 0;

		for (int sample = 0; sample < sample_count; ++sample) {
			const float xi = (sample + 0.5f) / sample_count;
			const float radius = std::sqrt(xi);
			const float angle = sample * golden_angle;
			const float normal_component = std::sqrt(1.0f - xi);
			const Eigen::Vector3f direction =
				tangent * (radius * std::cos(angle)) +
				bitangent * (radius * std::sin(angle)) + normal * normal_component;
			if (bvh.occluded(origin, direction, max_distance, ray_epsilon)) {
				++occluded_rays;
			}
		}

		AO(vertex, 0) = 1.0f - (float)occluded_rays / sample_count;
		if (progress != nullptr && (vertex % 256 == 0)) {
			progress->store(0.995f + 0.004f * (float)(vertex + 1) / vertex_count);
		}
	}
}

Mesh::Mesh(std::string filename, float scale, std::atomic<float> *progress, std::atomic<bool> *cancel_requested, std::atomic<int> *stage) {
	load_cancelled = false;
	if (stage != nullptr) {
		stage->store(1);
	}
	if (!read_obj(filename,vec_V,vec_N,vec_TC,vec_F, progress, cancel_requested)) {
		load_cancelled = true;
		return;
	}
	if (cancel_requested != nullptr && cancel_requested->load()) {
		load_cancelled = true;
		return;
	}

	if (vec_TC.empty()) {
		if (stage != nullptr) {
			stage->store(2);
		}
		buildBoxProjectionVT(progress, cancel_requested);
		if (cancel_requested != nullptr && cancel_requested->load()) {
			load_cancelled = true;
			return;
		}
	}

	if (progress != nullptr) progress->store(0.84f);
	has_vt_mapping = !vec_TC.empty();
	// printf("vec_V: %ld, vec_N: %ld, vec_TC: %ld, vec_F: %ld\n", vec_V.size(), vec_N.size(), vec_TC.size(), vec_F.size());

	struct comp {
		bool operator()(const Eigen::VectorXi& a, const Eigen::VectorXi& b) const {
			assert(a.size()==b.size());
			for(size_t i=0;i<a.size();++i) {
				if(a[i]<b[i]) return true;
				if(a[i]>b[i]) return false;
			}
			return false;
		}
	};

	// <vertex_id, [tc_id, norm_id, new_vertex_id]>
	std::map<int, std::map<Eigen::Vector2i, int, comp>> ver_rep_check;
	std::vector<Face> vec_F_cp = vec_F;
	int counter = (int)(vec_V.size());
	for (int i=0; i<vec_F_cp.size(); i++) {
		if (cancel_requested != nullptr && cancel_requested->load()) {
			load_cancelled = true;
			return;
		}
		for (int j=0; j<3; j++) {
			int cur_v = vec_F_cp[i]._vertex(j);
			if (ver_rep_check.find(cur_v) == ver_rep_check.end()) {
				std::map<Eigen::Vector2i, int, comp> new_map;
				if (vec_TC.size() != 0 && vec_N.size() != 0) new_map[Eigen::Vector2i(vec_F_cp[i]._normal(j), vec_F_cp[i]._tex_coord(j))] = cur_v;
				if (vec_TC.size() != 0 && vec_N.size() == 0) new_map[Eigen::Vector2i(-1, vec_F_cp[i]._tex_coord(j))] = cur_v;
				if (vec_TC.size() == 0 && vec_N.size() != 0) new_map[Eigen::Vector2i(vec_F_cp[i]._normal(j), -1)] = cur_v;
				ver_rep_check[cur_v] = new_map;
			} else {
				Eigen::Vector2i cur_vec;
				if (vec_TC.size() != 0 && vec_N.size() != 0) cur_vec = Eigen::Vector2i(vec_F_cp[i]._normal(j), vec_F_cp[i]._tex_coord(j));
				if (vec_TC.size() != 0 && vec_N.size() == 0) cur_vec = Eigen::Vector2i(-1, vec_F_cp[i]._tex_coord(j));
				if (vec_TC.size() == 0 && vec_N.size() != 0) cur_vec = Eigen::Vector2i(vec_F_cp[i]._normal(j), -1);

				if (ver_rep_check[cur_v].find(cur_vec) == ver_rep_check[cur_v].end()) {
					ver_rep_check[cur_v][cur_vec] = counter;
					counter = counter + 1;
					vec_V.push_back(vec_V[cur_v]);
				}
			}
		}
	}
	if (progress != nullptr) progress->store(0.88f);
	for (int i=0; i<vec_F_cp.size(); i++) {
		if (cancel_requested != nullptr && cancel_requested->load()) {
			load_cancelled = true;
			return;
		}
		for (int j=0; j<3; j++) {
			int cur_v = vec_F_cp[i]._vertex(j);
			Eigen::Vector2i cur_vec;
			if (vec_TC.size() != 0 && vec_N.size() != 0) cur_vec = Eigen::Vector2i(vec_F_cp[i]._normal(j), vec_F_cp[i]._tex_coord(j));
			if (vec_TC.size() != 0 && vec_N.size() == 0) cur_vec = Eigen::Vector2i(-1, vec_F_cp[i]._tex_coord(j));
			if (vec_TC.size() == 0 && vec_N.size() != 0) cur_vec = Eigen::Vector2i(vec_F_cp[i]._normal(j), -1);
			vec_F[i]._vertex(j) = ver_rep_check[cur_v][cur_vec];
		}
	}
	if (progress != nullptr) progress->store(0.91f);

	top_right = Eigen::Vector3f(0,0,0);
	bottom_left = Eigen::Vector3f(0,0,0);

	Eigen::MatrixXf mat_V; // mat_V
	mat_V.resize(vec_V.size(), 3);
	// printf("vec_V: %ld, vec_N: %ld, vec_TC: %ld, vec_F: %ld\n", vec_V.size(), vec_N.size(), vec_TC.size(), vec_F.size());

	for (int i=0; i<vec_V.size(); i++) {
		if (cancel_requested != nullptr && cancel_requested->load()) {
			load_cancelled = true;
			return;
		}
		mat_V.row(i) << vec_V[i](0), vec_V[i](1), vec_V[i](2);
		if (vec_V[i](0) > top_right(0)) {top_right(0) = vec_V[i](0);}
		if (vec_V[i](1) > top_right(1)) {top_right(1) = vec_V[i](1);}
		if (vec_V[i](2) > top_right(2)) {top_right(2) = vec_V[i](2);}

		if (vec_V[i](0) < bottom_left(0)) {bottom_left(0) = vec_V[i](0);}
		if (vec_V[i](1) < bottom_left(1)) {bottom_left(1) = vec_V[i](1);}
		if (vec_V[i](2) < bottom_left(2)) {bottom_left(2) = vec_V[i](2);}
	}
	if (progress != nullptr) progress->store(0.94f);

	buildV(scale);
	if (cancel_requested != nullptr && cancel_requested->load()) {
		load_cancelled = true;
		return;
	}
	if (progress != nullptr) progress->store(0.96f);
	buildN();
	if (cancel_requested != nullptr && cancel_requested->load()) {
		load_cancelled = true;
		return;
	}
	if (progress != nullptr) progress->store(0.98f);
	buildVN();
	if (cancel_requested != nullptr && cancel_requested->load()) {
		load_cancelled = true;
		return;
	}
	if (progress != nullptr) progress->store(0.995f);
	buildTC();
	if (cancel_requested != nullptr && cancel_requested->load()) {
		load_cancelled = true;
		return;
	}
	buildIndices();
	buildAO(progress, cancel_requested);
	if (cancel_requested != nullptr && cancel_requested->load()) {
		load_cancelled = true;
		return;
	}
	if (progress != nullptr) progress->store(1.0f);

	// printf("V: %ld, N: %ld, TC: %ld, T: %ld\n", V.rows(), N.rows(), TC.rows(), T.rows());
	start = 0;
}

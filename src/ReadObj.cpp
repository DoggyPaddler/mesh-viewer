#include "ReadObj.h"

void append(std::vector<int> f_,
			int & cur_total_vertex,
			std::vector<Face> & F) {
	if (f_.size() == 3) {
		Face new_face(f_.size(), Eigen::Vector3i(f_[0], f_[1], f_[2]));
		new_face.start = cur_total_vertex;
		cur_total_vertex += f_.size();
		F.push_back(new_face);

	} else if (f_.size() == 4) {
		Face new_face(3, Eigen::Vector3i(f_[0], f_[1], f_[2]));
		new_face.start = cur_total_vertex;
		cur_total_vertex += 3;
		F.push_back(new_face);

		Face new_face2(3, Eigen::Vector3i(f_[0], f_[2], f_[3]));
		new_face2.start = cur_total_vertex;
		cur_total_vertex += 3;
		F.push_back(new_face2);

	} else {
		std::vector<int> f_head = {f_.begin(), f_.begin() + 4};
		std::vector<int> f_rest = {f_.begin() + 3, f_.end()};
		f_rest.insert(f_rest.begin(), f_[0]);
		// printf("%lu\n", f_.size());
		append(f_head, cur_total_vertex, F);
		append(f_rest, cur_total_vertex, F);
	}
}

void append(std::vector<int> f_,
			std::vector<int> n_,
			int & cur_total_vertex,
			std::vector<Face> & F) {
	if (f_.size() == 3) {
		Face new_face(f_.size(), Eigen::Vector3i(f_[0], f_[1], f_[2]), Eigen::Vector3i(n_[0], n_[1], n_[2]));
		new_face.start = cur_total_vertex;
		cur_total_vertex += f_.size();
		F.push_back(new_face);

	} else if (f_.size() == 4) {
		Face new_face(3, Eigen::Vector3i(f_[0], f_[1], f_[2]), Eigen::Vector3i(n_[0], n_[1], n_[2]));
		new_face.start = cur_total_vertex;
		cur_total_vertex += 3;
		F.push_back(new_face);

		Face new_face2(3, Eigen::Vector3i(f_[0], f_[2], f_[3]), Eigen::Vector3i(n_[0], n_[2], n_[3]));
		new_face2.start = cur_total_vertex;
		cur_total_vertex += 3;
		F.push_back(new_face2);

	} else {
		std::vector<int> f_head = {f_.begin(), f_.begin() + 4};
		std::vector<int> f_rest = {f_.begin() + 3, f_.end()};
		f_rest.insert(f_rest.begin(), f_[0]);

		std::vector<int> n_head = {n_.begin(), n_.begin() + 4};
		std::vector<int> n_rest = {n_.begin() + 3, n_.end()};
		n_rest.insert(n_rest.begin(), n_[0]);
		// printf("%lu\n", f_.size());
		append(f_head, n_head, cur_total_vertex, F);
		append(f_rest, n_rest, cur_total_vertex, F);
	}
}

void append(std::vector<int> f_,
			std::vector<int> n_,
			std::vector<int> tc_,
			int & cur_total_vertex,
			std::vector<Face> & F) {
	if (f_.size() == 3) {
		Face new_face(f_.size(), Eigen::Vector3i(f_[0], f_[1], f_[2]), Eigen::Vector3i(n_[0], n_[1], n_[2]), Eigen::Vector3i(tc_[0], tc_[1], tc_[2]));
		new_face.start = cur_total_vertex;
		cur_total_vertex += f_.size();
		F.push_back(new_face);

	} else if (f_.size() == 4) {
		Face new_face(3, Eigen::Vector3i(f_[0], f_[1], f_[2]), Eigen::Vector3i(n_[0], n_[1], n_[2]), Eigen::Vector3i(tc_[0], tc_[1], tc_[2]));
		new_face.start = cur_total_vertex;
		cur_total_vertex += 3;
		F.push_back(new_face);

		Face new_face2(3, Eigen::Vector3i(f_[0], f_[2], f_[3]), Eigen::Vector3i(n_[0], n_[2], n_[3]), Eigen::Vector3i(tc_[0], tc_[2], tc_[3]));
		new_face2.start = cur_total_vertex;
		cur_total_vertex += 3;
		F.push_back(new_face2);

	} else {
		std::vector<int> f_head = {f_.begin(), f_.begin() + 4};
		std::vector<int> f_rest = {f_.begin() + 3, f_.end()};
		f_rest.insert(f_rest.begin(), f_[0]);

		std::vector<int> n_head = {n_.begin(), n_.begin() + 4};
		std::vector<int> n_rest = {n_.begin() + 3, n_.end()};
		n_rest.insert(n_rest.begin(), n_[0]);

		std::vector<int> tc_head = {tc_.begin(), tc_.begin() + 4};
		std::vector<int> tc_rest = {tc_.begin() + 3, tc_.end()};
		tc_rest.insert(tc_rest.begin(), tc_[0]);
		// printf("%lu\n", f_.size());
		append(f_head, n_head, tc_head, cur_total_vertex, F);
		append(f_rest, n_rest, tc_rest, cur_total_vertex, F);
	}
}

void append_vtc(std::vector<int> f_,
			std::vector<int> tc_,
			int & cur_total_vertex,
			std::vector<Face> & F) {
	if (f_.size() == 3) {
		Face new_face(f_.size(), Eigen::Vector3i(f_[0], f_[1], f_[2]), Eigen::Vector3i(tc_[0], tc_[1], tc_[2]), 1);
		new_face.start = cur_total_vertex;
		cur_total_vertex += f_.size();
		F.push_back(new_face);

	} else if (f_.size() == 4) {
		Face new_face(3, Eigen::Vector3i(f_[0], f_[1], f_[2]), Eigen::Vector3i(tc_[0], tc_[1], tc_[2]), 1);
		new_face.start = cur_total_vertex;
		cur_total_vertex += 3;
		F.push_back(new_face);

		Face new_face2(3, Eigen::Vector3i(f_[0], f_[2], f_[3]), Eigen::Vector3i(tc_[0], tc_[2], tc_[3]), 1);
		new_face2.start = cur_total_vertex;
		cur_total_vertex += 3;
		F.push_back(new_face2);

	} else {
		std::vector<int> f_head = {f_.begin(), f_.begin() + 4};
		std::vector<int> f_rest = {f_.begin() + 3, f_.end()};
		f_rest.insert(f_rest.begin(), f_[0]);

		std::vector<int> tc_head = {tc_.begin(), tc_.begin() + 4};
		std::vector<int> tc_rest = {tc_.begin() + 3, tc_.end()};
		tc_rest.insert(tc_rest.begin(), tc_[0]);
		// printf("%lu\n", f_.size());
		append_vtc(f_head, tc_head, cur_total_vertex, F);
		append_vtc(f_rest, tc_rest, cur_total_vertex, F);
	}
}

bool read_obj(const std::string obj_file_name,
		std::vector<Eigen::Vector3f> & V,
		std::vector<Eigen::Vector3f> & N,
		std::vector<Eigen::Vector2f> & TC,
		std::vector<Face> & F) {

	std::string v("v");
	std::string vn("vn");
	std::string vt("vt");
	std::string f("f");
	std::string tic_tac_toe("#");


	// std::vector<Eigen::Vector3f> V;
	// std::vector<Eigen::Vector3f> N;
	// std::vector<Eigen::Vector2f> TC;

	// std::vector<std::vector<int>> F;
	std::vector<std::vector<int>> FTC;
	std::vector<std::vector<int>> FN;

	FILE * obj_file = fopen(obj_file_name.c_str(),"r");
	if (NULL==obj_file) {
		fprintf(stderr,"IOError: %s could not be opened...\n", obj_file_name.c_str());
		return false;
	}

	char line[2048];
	char currentmaterialref[2048] = "";

	int line_no = 1;
	int previous_face_no = 0;
	int current_face_no = 0;
	int cur_total_vertex = 0;

	while (fgets(line, 2048, obj_file) != NULL) {
		char type[2048];

		// Read first word containing type
		if(sscanf(line, "%s", type) == 1) {
			char *l = &line[strlen(type)]; //rest of line right after type
			if(type == v) {
				float x,y,z;
				if (sscanf(l, "%g %g %g", &x,&y,&z) == 3) {
					Eigen::Vector3f vertex(x,y,z);
					V.push_back(vertex);
				}
			} else if (type == vn) {
				float x,y,z;
				if (sscanf(l, "%g %g %g", &x,&y,&z) == 3) {
					Eigen::Vector3f normal(x,y,z);
					N.push_back(normal);
				}
			} else if (type == vt) {
				float x,y;
				if (sscanf(l, "%g %g", &x,&y) == 2) {
					Eigen::Vector2f tex(x,y);
					TC.push_back(tex);
				}

			} else if(type == f) {
				char word[2048];
				int offset;

				std::vector<int> f_;
				std::vector<int> ftc_;
				std::vector<int> fn_;

				const auto & shift = [&V](const int i)->int {return i<0 ? i+V.size() : i-1;};
				const auto & shift_t = [&TC](const int i)->int {return i<0 ? i+TC.size() : i-1;};
				const auto & shift_n = [&N](const int i)->int {return i<0 ? i+N.size() : i-1;};

				while(sscanf(l,"%s%n",word,&offset) == 1) {
					l += offset;
					long int i,it,in;

					if (sscanf(word,"%ld/%ld/%ld",&i,&it,&in) == 3) {
						f_.push_back(shift(i));
						ftc_.push_back(shift_t(it));
						fn_.push_back(shift_n(in));
					} else if (sscanf(word,"%ld/%ld",&i,&it) == 2) {
						f_.push_back(shift(i));
						ftc_.push_back(shift_t(it));
					} else if (sscanf(word,"%ld//%ld",&i,&in) == 2) {
						f_.push_back(shift(i));
						fn_.push_back(shift_n(in));
					} else if (sscanf(word,"%ld",&i) == 1) {
						f_.push_back(shift(i));
					}
				}

				if(f_.size()>0 && fn_.size() == 0 && ftc_.size() == 0) {
					append(f_, cur_total_vertex, F);

					// FTC.push_back(ftc_);
					// FN.push_back(fn_);
					current_face_no++;
				} else if (f_.size()>0 && fn_.size() == f_.size() && ftc_.size() == 0) {
					append(f_, fn_, cur_total_vertex, F);
					current_face_no++;
				} else if (f_.size()>0 && fn_.size() == f_.size() && ftc_.size() == f_.size()) {
					append(f_, fn_, ftc_, cur_total_vertex, F);
					current_face_no++;
				} else if (f_.size()>0 && fn_.size() == 0 && ftc_.size() == f_.size()) {
					append_vtc(f_, ftc_, cur_total_vertex, F);
					current_face_no++;
				}
			}
		} else {
			// ignore empty line
		}
		line_no++;
	}

	fclose(obj_file);
	return true;
}

bool read_obj(const std::string obj_file_name,
		std::vector<Eigen::Vector3f> & V,
		std::vector<Eigen::Vector3f> & N,
		std::vector<Eigen::Vector2f> & TC,
		std::vector<std::vector<int>> & F,
		std::vector<std::vector<int>> & FTC,
		std::vector<std::vector<int>> & FN) {

	std::string v("v");
	std::string vn("vn");
	std::string vt("vt");
	std::string f("f");
	std::string tic_tac_toe("#");

	FILE * obj_file = fopen(obj_file_name.c_str(),"r");
	if (NULL==obj_file) {
		fprintf(stderr,"IOError: %s could not be opened...\n", obj_file_name.c_str());
		return false;
	}

	char line[2048];
	char currentmaterialref[2048] = "";

	int line_no = 1;
	int previous_face_no = 0;
	int current_face_no = 0;

	while (fgets(line, 2048, obj_file) != NULL) {
		char type[2048];
		// Read first word containing type
		if(sscanf(line, "%s", type) == 1) {
			char *l = &line[strlen(type)]; //rest of line right after type
			if(type == v) {
				float x,y,z;
				if (sscanf(l, "%g %g %g", &x,&y,&z) == 3) {
					// std::cout << "hello " << x << " " << y<< " " << z << std::endl;
					Eigen::Vector3f vertex(x,y,z);
					V.push_back(vertex);
				}
			} else if (type == vn) {
				// double x[3];
				float x,y,z;
				if (sscanf(l, "%g %g %g", &x,&y,&z) == 3) {
					Eigen::Vector3f normal(x,y,z);
					N.push_back(normal);
				}
			} else if (type == vt) {
				float x,y;
				if (sscanf(l, "%g %g", &x,&y) == 2) {
					Eigen::Vector2f tex(x,y);
					TC.push_back(tex);
				}

			} else if(type == f) {
				char word[2048];
				int offset;

				std::vector<int> f_;
				std::vector<int> ftc_;
				std::vector<int> fn_;

				const auto & shift = [&V](const int i)->int {return i<0 ? i+V.size() : i-1;};
				const auto & shift_t = [&TC](const int i)->int {return i<0 ? i+TC.size() : i-1;};
				const auto & shift_n = [&N](const int i)->int {return i<0 ? i+N.size() : i-1;};

				while(sscanf(l,"%s%n",word,&offset) == 1) {
					// std::cout << "hello " << word << std::endl;
					l += offset;
					long int i,it,in;

					if (sscanf(word,"%ld/%ld/%ld",&i,&it,&in) == 3) {
						f_.push_back(shift(i));
						ftc_.push_back(shift_t(it));
						fn_.push_back(shift_n(in));
					} else if (sscanf(word,"%ld/%ld",&i,&it) == 2) {
						f_.push_back(shift(i));
						ftc_.push_back(shift_t(it));
					} else if (sscanf(word,"%ld//%ld",&i,&in) == 2) {
						f_.push_back(shift(i));
						fn_.push_back(shift_n(in));
					} else if (sscanf(word,"%ld",&i) == 1) {
						f_.push_back(shift(i));
					}
				}

				if(
					(f_.size()>0 && fn_.size() == 0 && ftc_.size() == 0) ||
					(f_.size()>0 && fn_.size() == f_.size() && ftc_.size() == 0) ||
					(f_.size()>0 && fn_.size() == 0 && ftc_.size() == f_.size()) ||
					(f_.size()>0 && fn_.size() == f_.size() && ftc_.size() == f_.size())) {
					// No matter what add each type to lists so that lists are the
					// correct lengths
					F.push_back(f_);
					FTC.push_back(ftc_);
					FN.push_back(fn_);
					current_face_no++;
				}
			}
		} else {
			// ignore empty line
		}
		line_no++;
	}
	fclose(obj_file);
	return true;
}


#include "Editor.h"
#include <nanogui/common.h>
// #include "fileDialog.mm"
#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>
#include <objc/objc-runtime.h>
#include <iostream>


Editor::Editor(Eigen::Vector3f cam_init_pos, std::vector<std::string> obj_list, std::vector<std::string> tex_list) {
	cursor = new Cursor();
	resources = new ResourceManager();
	cam = new Camera(cam_init_pos);
	cam->view(0,0) = 400.0/600.0 * cam->view(1,1);
	cur_mesh = 0;
	cur_tex = 0;
	render_mode = 0;
	useTex = 0;

	window = glfwCreateWindow(600, 400, "3D Viewer", nullptr, nullptr);    // Create a GLFWwindow object
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeLimits(window, 600, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);

	gui.nanogui_init(window, obj_list, tex_list, &useTex);
}

void Editor::insert_mesh(std::string filename, float scale) {
	printf("%s\n", filename.c_str());
	Mesh new_mesh(filename, scale);

	float m1 = std::max(new_mesh.top_right(0) - new_mesh.bottom_left(0), new_mesh.top_right(1) - new_mesh.bottom_left(1));
	float m2 = std::max(m1, new_mesh.top_right(2) - new_mesh.bottom_left(2));
	new_mesh.V = new_mesh.V * 1.0/m2;

	int n = (int)mesh_vector.size();

	resources->addVAO(new VertexArrayObject());
	resources->addVBO(new VertexBufferObject());//v
	resources->addVBO(new VertexBufferObject());//n
	resources->addVBO(new VertexBufferObject());//tc
	resources->addVBO(new VertexBufferObject());//fn
	resources->addVBO(new VertexBufferObject());//tngt

	resources->addEBO(new ElementBufferObject());//ebo

	resources->_usedVAOs[n]->bind();
	resources->_usedVBOs[n*5]->update(new_mesh.V.transpose());
	resources->_usedVBOs[n*5+1]->update(new_mesh.VN.transpose());
	resources->_usedVBOs[n*5+2]->update(new_mesh.TC.transpose());
	resources->_usedVBOs[n*5+3]->update(new_mesh.N.transpose());
	resources->_usedVBOs[n*5+4]->update(new_mesh.T.transpose());
	resources->_usedEBOs[n]->update(new_mesh.Indices.transpose());

	mesh_vector.push_back(new_mesh);
}

void Editor::add_tex(std::string foldername) {
	int num_textures = (int)(resources->_usedTextures.size());
	// int tex_num_use = 
	resources->addTexture(new Texture());
    // resources->activeTexture(num_textures + 0);
    resources->_usedTextures[num_textures + 0]->bind();
    resources->_usedTextures[num_textures + 0]->update(foldername + "diffuse.png");
    // resources->_usedTextures[num_textures + 0]->bind();
    // glActiveTexture(GL_TEXTURE0 + 1);
    
    resources->addTexture(new Texture());
    // resources->activeTexture(num_textures + 1);
    resources->_usedTextures[num_textures + 1]->bind();
    resources->_usedTextures[num_textures + 1]->update(foldername + "normal.png");
    // resources->_usedTextures[num_textures + 1]->bind();
    // glActiveTexture(GL_TEXTURE0 + 2);
    
    resources->addTexture(new Texture());
    // resources->activeTexture(num_textures + 2);
    resources->_usedTextures[num_textures + 2]->bind();
    resources->_usedTextures[num_textures + 2]->update(foldername + "glossiness.png");
    // resources->_usedTextures[num_textures + 2]->bind();
    // glActiveTexture(GL_TEXTURE0 + 3);

    resources->addTexture(new Texture());
    // resources->activeTexture(num_textures + 3);
    resources->_usedTextures[num_textures + 3]->bind();
    resources->_usedTextures[num_textures + 3]->update(foldername + "specular.png");
    // resources->_usedTextures[num_textures + 3]->bind();

    // resources->addTexture(new Texture());
    // // resources->activeTexture(num_textures + 4);
    // resources->_usedTextures[num_textures + 4]->bind();
    // resources->_usedTextures[num_textures + 4]->update(foldername + "specular.png");
    // // resources->_usedTextures[num_textures + 4]->bind();

}

void Editor::setGUICallbacks() {
	gui.render_mode->setCallback([this](int a) {
		this->gui.render_mode->setSelectedIndex(a);
		nanogui::Button * buttonA = ((nanogui::Button *)gui.render_mode->popup()->children()[a]);
		buttonA->mouseEnterEvent({0,0}, false);
		this->render_mode = a;

		//std::cout << this->gui.render_mode->items().size() << std::endl;
		if (a == 0) {
			this->gui.glossiness->setVisible(false);
			this->gui.metallic->setVisible(false);
			this->gui.shineness->setVisible(true);
			this->gui.ambient->setVisible(true);
			this->gui.screen->performLayout();
			// this->gui.shineness->setVisible(true);
			//this->gui.nanoguiWindow->setSize(gui.nanoguiWindow->size() - Eigen::Vector2i(0,20));
		} else {
			this->gui.glossiness->setVisible(true);
			this->gui.metallic->setVisible(true);
			this->gui.shineness->setVisible(false);
			this->gui.ambient->setVisible(false);
			this->gui.screen->performLayout();
			//this->gui.nanoguiWindow->setSize(gui.nanoguiWindow->size() + Eigen::Vector2i(0,20));
		}
		// std::cout << this->gui.nanoguiWindow->size() << std::endl;
		// std::cout << this->gui.tb->size() << std::endl;
	});

	gui.texOrColor->setCallback([this](int a) {
		this->gui.texOrColor->setSelectedIndex(a);
		nanogui::Button * buttonA = ((nanogui::Button *)gui.texOrColor->popup()->children()[a]);
		buttonA->mouseEnterEvent({0,0}, false);
		// if (a==0) this->useTex = a;
		this->useTex = a;
		this->gui.ml2->setBackgroundColor(nanogui::Color(255, 255, 255, 255));

		int ml2_selectedIndex = this->gui.ml2->selectedIndex;
		this->gui.ml2->items[ml2_selectedIndex]->setBackgroundColor(nanogui::Color(255, 255, 255, 255));
        this->gui.ml2->items[ml2_selectedIndex]->setPushed(false);

        this->gui.ml2->setPushed(false);
	});
	gui.b1->setCallback([&]{cam->pause_or_resume();});
	gui.tb->setCallback([this](const int n) {
		this->resources->setFloat(0, "p", n);
	});
	nanogui::IntBox<int> * light_intensity_w = (nanogui::IntBox<int> *)gui.light_intensity->w;
	light_intensity_w->setCallback([this](const int n) {
		this->resources->setFloat(0, "light_intensity", n);
	});
	nanogui::IntBox<int> * gloss_w = (nanogui::IntBox<int> *)gui.glossiness->w;
	gloss_w->setCallback([this](const int n) {
		// std::cout << "hello" << std::endl;
		this->resources->setFloat(0, "kg", n);
	});
	auto tb_callback = [this](ColorController* cc, std::string s, int i, int n) {
		nanogui::Color c = cc->intbox_callback(i, n);
		this->resources->setFloat(0, s, c.r()*255.0, c.g()*255.0, c.b()*255.0);
	};
	auto cw_callback = [this](ColorController* cc, std::string s, const nanogui::Color &c) {
		cc->colorwheel_callback(c);
		this->resources->setFloat(0, s, c.r()*255.0, c.g()*255.0, c.b()*255.0);
	};
	auto open_mesh_callback = [&]() {
		std::string pathname = nanogui::file_dialog({{"obj", "Object File Format"}}, false);
		if (pathname.length() == 0) return;
		std::string file = pathname.substr(pathname.find_last_of("/") + 1, pathname.length());
		insert_mesh(pathname, 6);
		gui.ml->toolbox->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1,nanogui::Alignment::Maximum, 0, 0));
		gui.ml->addItem(file ,(int)gui.ml->names.size());
		gui.ml->names.push_back(file);
		gui.screen->performLayout();
	};
	auto open_tex_callback = [&]() {
		auto result = directory_dialog({{"", ""}}, false, true);
		std::string pathname = result.empty() ? "" : result.front();
		if (pathname.length() == 0) return;
		std::string file = pathname.substr(pathname.find_last_of("/") + 1, pathname.length());
		add_tex(pathname + "/");
		gui.ml2->toolbox->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1,nanogui::Alignment::Maximum, 0, 0));
		gui.ml2->addItem(file ,(int)gui.ml2->names.size(), &useTex);
		gui.ml2->names.push_back(file);
		gui.screen->performLayout();
	};

	gui.cc->tb_r->setCallback([this, tb_callback](const int n) {tb_callback(gui.cc, "diffuse", 0, n);});
	gui.cc->tb_g->setCallback([this, tb_callback](const int n) {tb_callback(gui.cc, "diffuse", 1, n);});
	gui.cc->tb_b->setCallback([this, tb_callback](const int n) {tb_callback(gui.cc, "diffuse", 2, n);});
	gui.cc->cw->setCallback([this, cw_callback](const nanogui::Color &c) {cw_callback(gui.cc, "diffuse", c);});

	gui.cc2->tb_r->setCallback([this, tb_callback](const int n) {tb_callback(gui.cc2, "specular", 0, n);});
	gui.cc2->tb_g->setCallback([this, tb_callback](const int n) {tb_callback(gui.cc2, "specular", 1, n);});
	gui.cc2->tb_b->setCallback([this, tb_callback](const int n) {tb_callback(gui.cc2, "specular", 2, n);});
	gui.cc2->cw->setCallback([this, cw_callback](const nanogui::Color &c) {cw_callback(gui.cc2, "specular", c);});

	gui.cc3->tb_r->setCallback([this, tb_callback](const int n) {tb_callback(gui.cc3, "ambient", 0, n);});
	gui.cc3->tb_g->setCallback([this, tb_callback](const int n) {tb_callback(gui.cc3, "ambient", 1, n);});
	gui.cc3->tb_b->setCallback([this, tb_callback](const int n) {tb_callback(gui.cc3, "ambient", 2, n);});
	gui.cc3->cw->setCallback([this, cw_callback](const nanogui::Color &c) {cw_callback(gui.cc3, "ambient", c);});

	gui.ml->open_mesh->setCallback(open_mesh_callback);
	gui.ml2->open_mesh->setCallback(open_tex_callback);
}

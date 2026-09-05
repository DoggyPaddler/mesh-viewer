#ifndef GUI_H
#define GUI_H

#define GL_SILENCE_DEPRECATION
#if defined(NANOGUI_GLAD)
    #if defined(NANOGUI_SHARED) && !defined(GLAD_GLAPI_EXPORT)
    #define GLAD_GLAPI_EXPORT
    #endif
    #include <glad/glad.h>
#else
    #if defined(__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #else
    #define GL_GLEXT_PROTOTYPES
    #endif
#endif

#include <nanogui/nanogui.h>
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/theme.h>
#include <Eigen/Core>
#include <Eigen/Dense>

class ColorController {
public:
    nanogui::PopupButton *pb;
    nanogui::ColorWheel *cw;
    nanogui::IntBox<int>* tb_r;
    nanogui::IntBox<int>* tb_g;
    nanogui::IntBox<int>* tb_b;

    ColorController(nanogui::Window *nanoguiWindow, nanogui::Color c);
    nanogui::Color intbox_callback(int channel, int n);
    void colorwheel_callback(nanogui::Color c);
};

class MeshList: public nanogui::PopupButton {
public:
    // nanogui::PopupButton *pb;
    std::vector<nanogui::Button *> items;
    std::vector<std::string> names;
    nanogui::Widget * toolbox;
    nanogui::Widget * parent;
    nanogui::Button *open_mesh;
    int selectedIndex;

    MeshList(nanogui::Widget *parent, std::vector<std::string> item_names);
    nanogui::Button * addItem(const std::string item_name, int i);
    virtual bool scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel) override;

protected:
    Eigen::Vector2f cumulative_rel;
};

class TexList: public nanogui::PopupButton {
public:
    // nanogui::PopupButton *pb;
    std::vector<nanogui::Button *> items;
    std::vector<std::string> names;
    nanogui::Widget * toolbox;
    nanogui::Widget * parent_window;
    nanogui::Widget * parent_button;
    nanogui::Button *open_mesh;
    int selectedIndex;
    int* useTex_ptr;
    bool selectable;

    TexList(nanogui::Widget *parent_window, nanogui::Widget *parent_button, std::vector<std::string> tex_names, int* useTex);
    nanogui::Button * addItem(const std::string item_name, int i, int* useTex);
    void setSelectable(bool enabled);
    virtual bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override;
    virtual bool scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel) override;

protected:
    Eigen::Vector2f cumulative_rel;
};

class Row {
public:
    nanogui::Label *lab;
    nanogui::Widget *w;

    Row(nanogui::Label *left, nanogui::Widget *right);
    Row(nanogui::Label *left, ColorController *right);
    void setVisible(bool visible);
};

class GUI {
	public:
		nanogui::Screen *screen = nullptr;
		nanogui::Window *nanoguiWindow;
		nanogui::ComboBox *render_mode;
        nanogui::ComboBox *texOrColor;
        nanogui::Button *b1;
        nanogui::IntBox<int>* tb;
        ColorController *cc;
        ColorController *cc2;
        ColorController *cc3;
        Row* light_intensity;
        Row* glossiness;
        Row* shineness;
        Row* metallic;
        Row* ambient;
        MeshList *ml;
        TexList *ml2;

		void nanogui_init(GLFWwindow* window, std::vector<std::string> obj_list, std::vector<std::string> tex_list, int* useTex);
        void combo_init(nanogui::ComboBox *combo_box);
};


#endif
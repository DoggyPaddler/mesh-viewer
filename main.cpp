// #include <GLFW/glfw3.h>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <iostream>
#include <SOIL/SOIL.h>

#include "GUI.h"
#include "Editor.h"

Editor* editor;

void sync_texture_ui_for_current_mesh(bool log_missing_vt) {
    const bool has_vt_mapping = editor->mesh_vector[editor->cur_mesh].has_vt_mapping;

    editor->gui.ml2->setSelectable(has_vt_mapping);

    if (!has_vt_mapping) {
        editor->useTex = 0;
        editor->gui.texOrColor->setSelectedIndex(0);

        if (log_missing_vt) {
            std::cout << "no vt mapping" << std::endl;
        }
    }

    editor->gui.screen->performLayout();
}

// Callback Functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    editor->gui.screen->resizeCallbackEvent(width, height);
    float aspect_ratio = float(height)/float(width);
    editor->cam->view(0,0) = aspect_ratio * editor->cam->view(1,1);
    glViewport(0, 0, width, height);
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    bool scroll = editor->gui.screen->scrollCallbackEvent(xoffset, yoffset);
    // printf("scroll res: %d\n", scroll);
    if (scroll) return;
    else {
        if (editor->cam->radius <= 2 && yoffset > 0) return;
        else if (editor->cam->radius >= 16 && yoffset < 0) return;

        if (editor->cam->radius - 0.2 * yoffset <= 2) editor->cam->radius = 2;
        else if (editor->cam->radius - 0.2 * yoffset >= 16) editor->cam->radius = 16;
        else editor->cam->radius = editor->cam->radius - 0.2 * yoffset;
    }
}

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
    editor->gui.screen->cursorPosCallbackEvent(xpos, ypos);
    int width, height;
    glfwGetCursorPos(window, &xpos, &ypos);
    glfwGetWindowSize(window, &width, &height);
    Eigen::Vector2f pixel(xpos, height-1-ypos); // pixel position
    // Track the mouse positions
    editor->cursor->p0 = editor->cursor->p1;
    editor->cursor->p1 = pixel;
    if (editor->cursor->right_key_pressed == 1) {
        float a = editor->cursor->p1(0) - editor->cursor->p0(0);
        float b = editor->cursor->p1(1) - editor->cursor->p0(1);
        editor->cam->alpha_shift += 0.5 * a * M_PI/90.0;
        editor->cam->beta_shift -= 0.5 * b * M_PI/90.0;
    }
    bool shift_pressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                         glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
    if (shift_pressed) {
        float dx = editor->cursor->p1(0) - editor->cursor->p0(0);
        float dy = editor->cursor->p1(1) - editor->cursor->p0(1);
        float movement_scale = editor->cam->radius / std::max(height, 1);
        Vector3f camera_right = editor->cam->camera.block<1, 3>(0, 0).transpose();
        Vector3f camera_up = editor->cam->camera.block<1, 3>(1, 0).transpose();
        editor->light_pos += movement_scale * (dx * camera_right + dy * camera_up);
        editor->gui.setLightPosition(editor->light_pos);
    }
}

void mouse_click_callback(GLFWwindow* window, int button, int action, int mods) {
    editor->gui.screen->mouseButtonCallbackEvent(button, action, mods);
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) editor->cursor->right_key_pressed = 1;
        else editor->cursor->right_key_pressed = 0;
    }
}

void setWindowCallbacks() {
    glfwSetCharCallback(editor->window,
        [](GLFWwindow *, unsigned int codepoint) {
        editor->gui.screen->charCallbackEvent(codepoint);
    });
    glfwSetDropCallback(editor->window,
        [](GLFWwindow *, int count, const char **filenames) {
        editor->gui.screen->dropCallbackEvent(count, filenames);
    });
    glfwSetKeyCallback(editor->window,
        [](GLFWwindow*, int key, int scancode, int action, int mods) {
        editor->gui.screen->keyCallbackEvent(key, scancode, action, mods);
    });
    glfwSetMouseButtonCallback(editor->window, mouse_click_callback); // Register the mouse callback
    glfwSetCursorPosCallback(editor->window, mouse_move_callback);    // Register the cursor move callback
    glfwSetFramebufferSizeCallback(editor->window, framebuffer_size_callback); // Update viewport
    glfwSetScrollCallback(editor->window, mouse_scroll_callback);
}

void renderer_draw() {
    editor->resources->_usedPrograms[0]->bind();
    glUniformMatrix4fv(editor->resources->_usedPrograms[0]->uniform("view"), 1, GL_FALSE, editor->cam->view.data());
    glUniformMatrix4fv(editor->resources->_usedPrograms[0]->uniform("camera"), 1, GL_FALSE, editor->cam->camera.data());
    Vector3f light_pos = editor->light_pos;
    glUniform3f(editor->resources->_usedPrograms[0]->uniform("light_pos"), light_pos(0), light_pos(1),light_pos(2));
    Vector3f camera_pos = editor->cam->cam_pos;
    glUniform3f(editor->resources->_usedPrograms[0]->uniform("camera_pos"), camera_pos(0), camera_pos(1), camera_pos(2));
    glUniform1i(editor->resources->_usedPrograms[0]->uniform("render_mode"), editor->render_mode);
    glUniform1i(editor->resources->_usedPrograms[0]->uniform("useTex"), editor->useTex);

    editor->resources->loadTex(editor->cur_tex);

    editor->resources->_usedVAOs[editor->cur_mesh]->bind();
    if (editor->cur_mesh != editor->gui.ml->selectedIndex) {
        editor->cur_mesh = editor->gui.ml->selectedIndex;
        editor->resources->loadMeshes(editor->cur_mesh);
        sync_texture_ui_for_current_mesh(true);
    }
    if (editor->cur_tex != editor->gui.ml2->selectedIndex) {
        editor->cur_tex = editor->gui.ml2->selectedIndex;
        // editor->resources->loadTex(editor->cur_tex);
    }
    // glDrawArrays(GL_TRIANGLES, 0, editor->mesh_vector[editor->cur_mesh].vec_F.size() * 3);
    // editor->resources->_usedEBOs[editor->cur_mesh]->bind();
    glDrawElements(GL_TRIANGLES, 3 * editor->mesh_vector[editor->cur_mesh].Indices.rows(), GL_UNSIGNED_INT, 0);
}

int main(int /* argc */, char ** /* argv */) {
    glfwInit();
    glfwSetTime(0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);

    std::vector<std::string> obj_list = {"../models/bunny2.obj", "../models/sample2.obj", "../models/sample1.obj"};
    std::vector<std::string> tex_list = {"../textures/JT-19228", "../textures/black_Lc1", "../textures/fabric_0034", 
                    "../textures/black_L10", "../textures/st_brown_L1", "../textures/orange_L3", "../textures/metal10"}; // "../textures/JT-19228", 

    editor = new Editor(Eigen::Vector3f(12.0, 0.0, 0.0), obj_list, tex_list);
    editor->setGUICallbacks();
    // editor->gui.nanogui_init(editor->window);

    editor->resources->addProgram(new Program(),"../shader/vertex_shader.glsl","../shader/fragment_shader.glsl","outColor");
    editor->resources->_usedPrograms[0]->bind();

    for (std::string obj_file : obj_list) editor->insert_mesh(obj_file, 6);
    for (std::string tex_dir : tex_list) editor->add_tex(tex_dir + "/");
    // std::string file = pathname.substr(pathname.find_last_of("/") + 1, pathname.length());

    Eigen::MatrixXf m = Eigen::MatrixXf::Identity(4, 4);
    glUniformMatrix4fv(editor->resources->_usedPrograms[0]->uniform("model"), 1, GL_FALSE, m.data()); 
    glUniformMatrix4fv(editor->resources->_usedPrograms[0]->uniform("proj"), 1, GL_FALSE, editor->cam->persp.data());
    glUniform3f(editor->resources->_usedPrograms[0]->uniform("diffuse"), 110,100,160);
    glUniform3f(editor->resources->_usedPrograms[0]->uniform("specular"), 100,100,100);
    glUniform3f(editor->resources->_usedPrograms[0]->uniform("ambient"), 40,40,40);
    glUniform3f(editor->resources->_usedPrograms[0]->uniform("pbr_ambient"), 20,20,20);
    glUniform1f(editor->resources->_usedPrograms[0]->uniform("p"), 100);
    glUniform1f(editor->resources->_usedPrograms[0]->uniform("kg"), 50);
    glUniform1f(editor->resources->_usedPrograms[0]->uniform("light_intensity"), 50.0f);
    glUniform1i(editor->resources->_usedPrograms[0]->uniform("render_mode"), editor->render_mode);
    glUniform1i(editor->resources->_usedPrograms[0]->uniform("useTex"), 0);
    glUniformMatrix4fv(editor->resources->_usedPrograms[0]->uniform("view"), 1, GL_FALSE, editor->cam->view.data());

    TimePoint t_start = TimePoint();
    // Game loop
    glClearColor(0.9f, 0.9f, 0.9f, 0.4f);
    glEnable(GL_DEPTH_TEST);

    setWindowCallbacks();

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    editor->cam->update_camera(std::chrono::high_resolution_clock::now(), t_start);
    editor->resources->loadMeshes(0);
    editor->resources->loadTex(0);
    sync_texture_ui_for_current_mesh(false);
    // editor->resources->loadTex(1);

    while (!glfwWindowShouldClose(editor->window)) {
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        editor->cam->update_camera(std::chrono::high_resolution_clock::now(), t_start);
        editor->poll_mesh_upload();
        // Draw triangles 

        renderer_draw();
        editor->gui.screen->drawWidgets();
        glfwSwapBuffers(editor->window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

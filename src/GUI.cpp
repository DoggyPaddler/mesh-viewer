#include "GUI.h"
#include <algorithm>

nanogui::Theme *textbox_theme;
nanogui::Theme *button_theme;
nanogui::Theme *ml_item_theme;

void default_theme_init(nanogui::Theme *t) {
    t->mWindowCornerRadius = 3;
    t->mWindowDropShadowSize = 0;
    t->mButtonCornerRadius = 3;
    t->mTabBorderWidth = 30;
    t->mWindowHeaderGradientTop = nanogui::Color(200, 200, 200, 0);
    t->mWindowHeaderGradientBot = nanogui::Color(200, 200, 200, 0);
    t->mTransparent = nanogui::Color(255, 255, 255, 0);
    t->mWindowFillUnfocused = nanogui::Color(180, 180, 180, 150);
    t->mWindowFillFocused = nanogui::Color(180, 180, 180, 150);
    t->mWindowHeaderSepTop = nanogui::Color(200, 200, 200, 0);
    t->mWindowHeaderSepBot = nanogui::Color(200, 200, 200, 0);
    t->mWindowTitleUnfocused = nanogui::Color(0, 0);
    t->mWindowTitleFocused = nanogui::Color(0, 0);
    t->mWindowPopup = nanogui::Color(180, 180, 180, 150);
    t->mFontBold = 3;
    t->mTextColorShadow = nanogui::Color(0, 0, 0, 0);
    t->mDropShadow = nanogui::Color(0, 0, 0, 0);
    t->mWindowHeaderHeight = 8;
    t->mStandardFontSize = 12;
    t->mButtonFontSize = 12;
    t->mBorderDark = nanogui::Color(255, 255, 255, 0);
    t->mBorderLight = nanogui::Color(255, 255, 255, 0);
    t->mButtonGradientTopPushed = nanogui::Color(180, 180, 180, 255);
    t->mButtonGradientBotPushed = nanogui::Color(180, 180, 180, 255);
    t->mButtonGradientTopFocused = nanogui::Color(180, 180, 180, 255);
    t->mButtonGradientBotFocused = nanogui::Color(180, 180, 180, 255);
    t->mButtonGradientTopUnfocused = nanogui::Color(255, 255, 255, 255);
    t->mButtonGradientBotUnfocused = nanogui::Color(255, 255, 255, 255);
    t->mTextColor = nanogui::Color(0, 255);
    t->mPopupChevronRightIcon = ENTYPO_ICON_CHEVRON_RIGHT;
    t->mPopupChevronLeftIcon = ENTYPO_ICON_CHEVRON_LEFT;
}

void textbox_theme_init(nanogui::Theme *textbox_theme){
    textbox_theme->mTextColor = nanogui::Color(0, 255);
    textbox_theme->mButtonFontSize = 12;
    textbox_theme->mWindowCornerRadius = 3;
    textbox_theme->mButtonCornerRadius = 3;
    textbox_theme->mBorderDark = nanogui::Color(0, 255);
    textbox_theme->mBorderLight = nanogui::Color(0, 255);
    textbox_theme->mTextColor = nanogui::Color(0, 255);
    textbox_theme->mDisabledTextColor = nanogui::Color(0, 255);
}

void button_theme_init(nanogui::Theme *button_theme) {
    button_theme->mTextColor = nanogui::Color(255, 255);
    button_theme->mButtonFontSize = 12;
    // button_theme->mDropShadow = nanogui::Color(0, 0, 0, 0);
    button_theme->mBorderDark = nanogui::Color(255, 255, 255, 0);
    button_theme->mBorderLight = nanogui::Color(255, 255, 255, 0);
    button_theme->mButtonCornerRadius = 3;
}

MeshList::MeshList(nanogui::Widget *parent, std::vector<std::string> item_names) 
    : nanogui::PopupButton(parent, item_names[0]), selectedIndex(0){
    // pb = new nanogui::PopupButton(parent, item_names[selectedIndex]);
    this->setTextColor(nanogui::Color(0, 255));
    this->setFixedSize(Eigen::Vector2i(120,20));
    nanogui::Popup *popup = this->popup();
    this->parent = parent;
    popup->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1,nanogui::Alignment::Middle, 10, 0));

    toolbox = new nanogui::Widget(popup);
    toolbox->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1,nanogui::Alignment::Maximum, 0, 0));
    names = item_names;
    cumulative_rel = Eigen::Vector2f(0,0);

    for (int i=0; i<item_names.size(); i++) {
        MeshList::addItem(item_names[i], i);
    }
    nanogui::Widget *tmp = new nanogui::Widget(popup);
    tmp->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1,nanogui::Alignment::Maximum, 0, 0));
    open_mesh = new nanogui::Button(tmp, "Open");
    open_mesh->setBackgroundColor(nanogui::Color(74, 74, 74, 255));
    open_mesh->setTheme(button_theme);
    open_mesh->setFixedSize(Eigen::Vector2i(120,20));

    if (!items.empty()) {
        selectItem(this->selectedIndex, false);
    }
    // items[selectedIndex]->setBackgroundColor(nanogui::Color(180, 180, 180, 255));
}

nanogui::Button * MeshList::addItem(const std::string item_name, int i) {
    nanogui::Widget *row = new nanogui::Widget(toolbox);
    row->setFixedSize(Eigen::Vector2i(120, 20));

    nanogui::Button *b = new nanogui::Button(row, item_name);
    b->setPosition(Eigen::Vector2i(0, 0));
    b->setBackgroundColor(nanogui::Color(255, 255, 255, 255));
    b->setFixedSize(Eigen::Vector2i(120,20));
    b->setCallback([this, b]{
        auto it = std::find(items.begin(), items.end(), b);
        if (it == items.end()) {
            return;
        }
        int index = (int)std::distance(items.begin(), it);
        selectItem(index, true);
    });

    nanogui::Button *delete_button = new nanogui::Button(row, "x");
    delete_button->setPosition(Eigen::Vector2i(102, 2));
    delete_button->setBackgroundColor(nanogui::Color(255, 255, 255, 255));
    delete_button->setTextColor(nanogui::Color(0, 255));
    delete_button->setFontSize(10);
    delete_button->setFixedSize(Eigen::Vector2i(16, 16));
    delete_button->setCallback([this, delete_button] {
        auto it = std::find(delete_items.begin(), delete_items.end(), delete_button);
        if (it == delete_items.end()) {
            return;
        }
        int index = (int)std::distance(delete_items.begin(), it);
        if (onDeleteRequest) {
            onDeleteRequest(index);
        }
    });

    item_rows.push_back(row);
    items.push_back(b);
    delete_items.push_back(delete_button);
    return b;
}

void MeshList::selectItem(int index, bool close_popup) {
    if (items.empty()) {
        selectedIndex = 0;
        setCaption("");
        return;
    }

    index = std::max(0, std::min(index, (int)items.size() - 1));

    for (int k = 0; k < (int)items.size(); ++k) {
        const nanogui::Color item_color = (k == index)
            ? nanogui::Color(180, 180, 180, 255)
            : nanogui::Color(255, 255, 255, 255);
        items[k]->setBackgroundColor(item_color);
        delete_items[k]->setBackgroundColor(item_color);
        items[k]->setPushed(k == index);
    }

    selectedIndex = index;
    setCaption(names[selectedIndex]);
    cumulative_rel(1) = selectedIndex * 6.0;

    if (close_popup) {
        popup()->setVisible(false);
        setPushed(false);
    }
}

void MeshList::removeItem(int index) {
    if (index < 0 || index >= (int)items.size()) {
        return;
    }

    nanogui::Widget *row = item_rows[index];
    row->setVisible(false);

    item_rows.erase(item_rows.begin() + index);
    items.erase(items.begin() + index);
    delete_items.erase(delete_items.begin() + index);
    names.erase(names.begin() + index);

    if (items.empty()) {
        selectedIndex = 0;
        setCaption("");
        popup()->setVisible(false);
        setPushed(false);
        return;
    }

    int next = selectedIndex;
    if (next >= (int)items.size()) {
        next = (int)items.size() - 1;
    }
    if (index < selectedIndex) {
        next = selectedIndex - 1;
    }

    selectItem(next, false);
}

void MeshList::setDeleteCallback(std::function<void(int)> callback) {
    onDeleteRequest = callback;
}

bool MeshList::scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel) {
    if (names.empty()) {
        return true;
    }
    if (cumulative_rel.y() + rel.y() > 0 && cumulative_rel.y() + rel.y() < names.size() * 6.0) {
        cumulative_rel = cumulative_rel + rel;
    }
    // printf("cumulative_rel.y: %f\n", cumulative_rel.y());
    int next_selectedIndex = std::min((int)(cumulative_rel.y()/6.0), (int)(names.size()-1));
    if (next_selectedIndex != selectedIndex) {
        selectItem(next_selectedIndex, false);
    }
    return Widget::scrollEvent(p, rel);
}

TexList::TexList(nanogui::Widget *parent_window, nanogui::Widget *parent_button, std::vector<std::string> tex_names, int * useTex) 
    :nanogui::PopupButton(parent_window, tex_names[0]), selectedIndex(0), selectable(true){
    // pb = new nanogui::PopupButton(parent, tex_names[selectedIndex]);
    this->setTextColor(nanogui::Color(0, 255));
    this->setFixedSize(Eigen::Vector2i(120,20));
    nanogui::Popup *popup = this->popup();
    this->parent_window = parent_window;
    this->parent_button = parent_button;
    popup->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1,nanogui::Alignment::Middle, 10, 0));

    toolbox = new nanogui::Widget(popup);
    toolbox->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1,nanogui::Alignment::Maximum, 0, 0));
    names = tex_names;
    cumulative_rel = Eigen::Vector2f(0,0);

    useTex_ptr = useTex;

    for (int i=0; i<tex_names.size(); i++) {
        TexList::addItem(tex_names[i], i, useTex);
    }
    nanogui::Widget *tmp = new nanogui::Widget(popup);
    tmp->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1,nanogui::Alignment::Maximum, 0, 0));
    open_mesh = new nanogui::Button(tmp, "Open");
    open_mesh->setBackgroundColor(nanogui::Color(74, 74, 74, 255));
    open_mesh->setTheme(button_theme);
    open_mesh->setFixedSize(Eigen::Vector2i(120,20));

    if (!items.empty()) {
        selectItem(this->selectedIndex, false);
    }
    // this->setBackgroundColor(nanogui::Color(180, 100, 0, 255));
}

void TexList::selectItem(int index, bool close_popup) {
    if (items.empty()) {
        selectedIndex = 0;
        setCaption("");
        return;
    }

    index = std::max(0, std::min(index, (int)items.size() - 1));

    for (int k = 0; k < (int)items.size(); ++k) {
        const nanogui::Color item_color = (k == index)
            ? nanogui::Color(180, 180, 180, 255)
            : nanogui::Color(255, 255, 255, 255);
        items[k]->setBackgroundColor(item_color);
        delete_items[k]->setBackgroundColor(item_color);
        items[k]->setPushed(k == index);
    }

    selectedIndex = index;
    cumulative_rel(1) = selectedIndex * 6.0;

    if (close_popup) {
        popup()->setVisible(false);
        setPushed(false);
    }
}

void TexList::setSelectable(bool enabled) {
    selectable = enabled;

    if (!selectable) {
        popup()->setVisible(false);
        setPushed(false);
    }

    setTextColor(selectable ? nanogui::Color(0, 255) : nanogui::Color(120, 255));
    setBackgroundColor(selectable ? nanogui::Color(255, 255, 255, 255) : nanogui::Color(210, 210, 210, 255));

    for (int index = 0; index < items.size(); ++index) {
        nanogui::Button *item = items[index];
        nanogui::Button *delete_item = delete_items[index];
        const bool is_selected = index == selectedIndex;
        item->setTextColor(selectable ? nanogui::Color(0, 255) : nanogui::Color(120, 255));
        delete_item->setTextColor(selectable ? nanogui::Color(0, 255) : nanogui::Color(120, 255));
        if (selectable) {
            nanogui::Color c = is_selected ? nanogui::Color(180, 180, 180, 255) : nanogui::Color(255, 255, 255, 255);
            item->setBackgroundColor(c);
            delete_item->setBackgroundColor(c);
        } else {
            item->setBackgroundColor(nanogui::Color(210, 210, 210, 255));
            delete_item->setBackgroundColor(nanogui::Color(210, 210, 210, 255));
            item->setPushed(false);
        }
    }

    open_mesh->setTextColor(selectable ? nanogui::Color(255, 255) : nanogui::Color(180, 255));
    open_mesh->setBackgroundColor(selectable ? nanogui::Color(74, 74, 74, 255) : nanogui::Color(140, 140, 140, 255));
}

nanogui::Button * TexList::addItem(const std::string item_name, int i, int *uesTex) {
    nanogui::Widget *row = new nanogui::Widget(toolbox);
    row->setFixedSize(Eigen::Vector2i(120, 20));

    nanogui::Button *b = new nanogui::Button(row, item_name);
    b->setPosition(Eigen::Vector2i(0, 0));
    b->setBackgroundColor(nanogui::Color(255, 255, 255, 255));
    b->setFixedSize(Eigen::Vector2i(120,20));
    b->setCallback([this, b, uesTex]{
        if (!selectable) {
            return;
        }

        auto it = std::find(items.begin(), items.end(), b);
        if (it == items.end()) {
            return;
        }
        int index = (int)std::distance(items.begin(), it);
        selectItem(index, true);

        this->setBackgroundColor(nanogui::Color(180, 180, 180, 255));
        *uesTex = 1;
        nanogui::Button * buttonA = ((nanogui::Button *)parent_window->children()[0]);
        buttonA->setPushed(false);
        ((nanogui::Button *)parent_button)->setCaption(b->caption());
    });

    nanogui::Button *delete_button = new nanogui::Button(row, "x");
    delete_button->setPosition(Eigen::Vector2i(102, 2));
    delete_button->setBackgroundColor(nanogui::Color(255, 255, 255, 255));
    delete_button->setTextColor(nanogui::Color(0, 255));
    delete_button->setFontSize(10);
    delete_button->setFixedSize(Eigen::Vector2i(16, 16));
    delete_button->setCallback([this, delete_button] {
        if (!selectable) {
            return;
        }
        auto it = std::find(delete_items.begin(), delete_items.end(), delete_button);
        if (it == delete_items.end()) {
            return;
        }
        int index = (int)std::distance(delete_items.begin(), it);
        if (onDeleteRequest) {
            onDeleteRequest(index);
        }
    });

    item_rows.push_back(row);
    items.push_back(b);
    delete_items.push_back(delete_button);
    return b;
}

void TexList::removeItem(int index) {
    if (index < 0 || index >= (int)items.size()) {
        return;
    }

    nanogui::Widget *row = item_rows[index];
    row->setVisible(false);

    item_rows.erase(item_rows.begin() + index);
    items.erase(items.begin() + index);
    delete_items.erase(delete_items.begin() + index);
    names.erase(names.begin() + index);

    if (items.empty()) {
        selectedIndex = 0;
        setCaption("");
        popup()->setVisible(false);
        setPushed(false);
        return;
    }

    int next = selectedIndex;
    if (next >= (int)items.size()) {
        next = (int)items.size() - 1;
    }
    if (index < selectedIndex) {
        next = selectedIndex - 1;
    }

    selectItem(next, false);
}

void TexList::setDeleteCallback(std::function<void(int)> callback) {
    onDeleteRequest = callback;
}

bool TexList::mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) {
    if (!selectable) {
        popup()->setVisible(false);
        setPushed(false);
        return true;
    }

    return PopupButton::mouseButtonEvent(p, button, down, modifiers);
}

bool TexList::scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel) {
    if (!selectable) {
        return true;
    }

    *useTex_ptr = 1;
    if (cumulative_rel.y() + rel.y() > 0 && cumulative_rel.y() + rel.y() < names.size() * 6.0) {
        cumulative_rel = cumulative_rel + rel;
    }
    // printf("cumulative_rel.y: %f\n", cumulative_rel.y());
    int next_selectedIndex = std::min((int)(cumulative_rel.y()/6.0), (int)(names.size()-1));
    if (next_selectedIndex != selectedIndex) {
        selectItem(next_selectedIndex, false);
    }
    return Widget::scrollEvent(p, rel);
}
Row::Row(nanogui::Label *left, nanogui::Widget *right) {
    lab = left;
    w = right;
}

Row::Row(nanogui::Label *left, ColorController *right) {
    lab = left;
    w = right->pb;
}

void Row::setVisible(bool visible) {
    this->lab->setVisible(visible);
    this->w->setVisible(visible);
}

ColorController::ColorController(nanogui::Window *nanoguiWindow, nanogui::Color c) {
    // nanogui::Label* lab = new nanogui::Label(nanoguiWindow, "Diffuse", "sans");
    nanogui::Label* lab;
    pb = new nanogui::PopupButton(nanoguiWindow, "");
    pb->setTextColor(nanogui::Color(0, 255));
    pb->setFixedSize(Eigen::Vector2i(120,20));
    pb->setBackgroundColor(c);
    pb->setTextColor(c.contrastingColor());
    nanogui::Popup *popup = pb->popup();
    popup->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1,nanogui::Alignment::Middle, 10, 0));

    nanogui::Widget *tools;
    tools = new nanogui::Widget(popup);
    tools->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1,nanogui::Alignment::Maximum, 5, 0));
    cw = new nanogui::ColorWheel(tools, c);

    tools = new nanogui::Widget(popup);
    nanogui::GridLayout *layout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2,nanogui::Alignment::Middle, 0, 0);
    layout->setColAlignment({nanogui::Alignment::Maximum, nanogui::Alignment::Fill});
    tools->setLayout(layout);

    lab = new nanogui::Label(tools, "Red", "sans");
    tb_r = new nanogui::IntBox<int>(tools);
    tb_r->setEditable(true);
    tb_r->setFixedSize(Eigen::Vector2i(40, 20));
    tb_r->setValue((int) (c.r()* 255.0f));
    // tb_r->setTheme(textbox_theme);
    tb_r->setFontSize(12);
    tb_r->setFormat("[0-9]*");
    tb_r->setMinMaxValues(0, 255);

    lab = new nanogui::Label(tools, "Green", "sans");
    tb_g = new nanogui::IntBox<int>(tools);
    tb_g->setEditable(true);
    tb_g->setFixedSize(Eigen::Vector2i(40, 20));
    tb_g->setValue((int) (c.g()* 255.0f));
    // tb_g->setDefaultValue("0.0");
    // tb_g->setTheme(textbox_theme);
    tb_g->setFontSize(12);
    tb_g->setFormat("[0-9]*");
    tb_g->setMinMaxValues(0, 255);

    lab = new nanogui::Label(tools, "Blue", "sans");
    tb_b = new nanogui::IntBox<int>(tools);
    tb_b->setEditable(true);
    tb_b->setFixedSize(Eigen::Vector2i(40, 20));
    tb_b->setValue((int) (c.b()* 255.0f));
    // tb_b->setTheme(textbox_theme);
    tb_b->setFontSize(12);
    tb_b->setFormat("[0-9]*");
    tb_b->setMinMaxValues(0, 255);
}

nanogui::Color ColorController::intbox_callback(int channel, int n) {
    nanogui::Color c = cw->color();
    int clipped_n = 255;
    if (n < 255) 
        clipped_n = n;

    if (channel == 0) 
        c = nanogui::Color(clipped_n, (int) (c.g()* 255.0f), (int) (c.b()* 255.0f), 255);
    else if (channel == 1) 
        c = nanogui::Color((int) (c.r()* 255.0f), clipped_n, (int) (c.b()* 255.0f), 255);
    else 
        c = nanogui::Color((int) (c.r()* 255.0f), (int) (c.g()* 255.0f), clipped_n, 255);

    cw->setColor(c);
    pb->setBackgroundColor(c);
    return c;
}

void ColorController::colorwheel_callback(nanogui::Color c) {
    pb->setBackgroundColor(c);
    pb->setTextColor(c.contrastingColor());
    int red = (int) (c.r() * 255.0f);
    tb_r->setValue(red);
    int green = (int) (c.g() * 255.0f);
    tb_g->setValue(green);
    int blue = (int) (c.b() * 255.0f);
    tb_b->setValue(blue);
}


void GUI::combo_init(nanogui::ComboBox *combo_box) {
    combo_box->setSelectedIndex(0);
    combo_box->setTextColor(nanogui::Color(0, 255));
    combo_box->setFixedSize(Eigen::Vector2i(120,20));
    combo_box->setCallback([](int a) {});
    // p = combo_box->popup();
    const std::vector<nanogui::Widget *> &children3 = combo_box->popup()->children();
    for (int i=0; i<combo_box->items().size(); i++) {
        // ((nanogui::Button *) children3[i])->setBackgroundColor(nanogui::Color(255, 255, 255, 255));
        ((nanogui::Button *) children3[i])->setTextColor(nanogui::Color(0, 255));
        ((nanogui::Button *) children3[i])->setFixedSize(Eigen::Vector2i(120,20));
    }
    nanogui::Popup* p = combo_box->popup();
    nanogui::BoxLayout *box_layout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Middle, 10, 0);

    p->setLayout(box_layout);
    p->setAnchorHeight(26);
}

void GUI::setUploadActive(bool active) {
    nanoguiWindow->setVisible(!active);
    uploadWindow->setVisible(active);
    if (active) {
        upload_progress->setValue(0.0f);
        upload_stage_label->setCaption("processing OBJ");
        screen->performLayout();
        Eigen::Vector2i s = screen->size();
        Eigen::Vector2i w = uploadWindow->size();
        uploadWindow->setPosition(Eigen::Vector2i((s.x() - w.x()) / 2, (s.y() - w.y()) / 2));
    }
    screen->performLayout();
}

void GUI::setUploadProgress(float p) {
    float clamped = std::max(0.0f, std::min(1.0f, p));
    upload_progress->setValue(clamped);
}

void GUI::setUploadStage(const std::string &stage_text) {
    upload_stage_label->setCaption(stage_text);
}

void GUI::setLightPosition(const Eigen::Vector3f &position) {
    light_position_x->setValue(position.x());
    light_position_y->setValue(position.y());
    light_position_z->setValue(position.z());
}

void GUI::nanogui_init(GLFWwindow* window, std::vector<std::string> obj_list, std::vector<std::string> tex_list, int* useTex) {
    auto name_extractor = [](std::string p) -> std::string {return p.substr(p.find_last_of("/") + 1, p.length());};
    std::transform(obj_list.begin(), obj_list.end(), obj_list.begin(), name_extractor);
    std::transform(tex_list.begin(), tex_list.end(), tex_list.begin(), name_extractor);

	screen = new nanogui::Screen();
    screen->initialize(window, true);
    nanoguiWindow = new nanogui::Window(screen, "Control Panel");
    nanoguiWindow->setPosition(Eigen::Vector2i(20,20));

    // theme
    nanogui::Theme *t = new nanogui::Theme(screen->nvgContext());
    default_theme_init(t);
    screen->setTheme(t);

    textbox_theme = new nanogui::Theme(screen->nvgContext());
    textbox_theme_init(textbox_theme);

    button_theme = new nanogui::Theme(screen->nvgContext());
    button_theme_init(button_theme);

    //layout
    nanogui::GridLayout *layout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2,nanogui::Alignment::Middle, 15, 0);
    layout->setColAlignment({nanogui::Alignment::Maximum, nanogui::Alignment::Fill});
    nanoguiWindow->setLayout(layout);

    uploadWindow = new nanogui::Window(screen, "Uploading OBJ");
    uploadWindow->setPosition(Eigen::Vector2i(220, 20));
    uploadWindow->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Middle, 8, 8));
    upload_stage_label = new nanogui::Label(uploadWindow, "processing OBJ", "sans");
    upload_stage_label->setFixedWidth(220);
    upload_progress = new nanogui::ProgressBar(uploadWindow);
    upload_progress->setFixedSize(Eigen::Vector2i(220, 10));
    upload_progress->setValue(0.0f);
    cancel_upload = new nanogui::Button(uploadWindow, "Cancel");
    cancel_upload->setBackgroundColor(nanogui::Color(74, 74, 74, 255));
    cancel_upload->setTheme(button_theme);
    cancel_upload->setFixedSize(Eigen::Vector2i(220, 24));
    uploadWindow->setVisible(false);

    nanogui::Label* lab;

    lab = new nanogui::Label(nanoguiWindow, "Render Options", "sans");
    render_mode = new nanogui::ComboBox(nanoguiWindow, {"Blinn-Phong", "PBR Basic"});
    combo_init(render_mode);

    lab = new nanogui::Label(nanoguiWindow, "Textures", "sans");
    texOrColor = new nanogui::ComboBox(nanoguiWindow, {"Colorwheel"});
    // texOrColor = new nanogui::ComboBox(nanoguiWindow, {"Colorwheel"});
    combo_init(texOrColor);
    
    ml2 = new TexList(texOrColor->popup(), texOrColor, tex_list, useTex);
    ml2->setCaption("Texture");
    // ml2->setTheme(ml_item_theme);

    lab = new nanogui::Label(nanoguiWindow, "Assets", "sans");
    ml = new MeshList(nanoguiWindow, obj_list);
    // ml->items[0]->setBackgroundColor(nanogui::Color(74, 74, 74, 255));

    lab = new nanogui::Label(nanoguiWindow, "Rotate Camera", "sans");
    b1 = new nanogui::Button(nanoguiWindow, "Pause/Resume");
    b1->setBackgroundColor(nanogui::Color(74, 74, 74, 255));
    b1->setTheme(button_theme);
    b1->setFixedSize(Eigen::Vector2i(120,20));

    lab = new nanogui::Label(nanoguiWindow, "Light", "sans");
    light_popup = new nanogui::PopupButton(nanoguiWindow, "Light");
    light_popup->setTextColor(nanogui::Color(0, 255));
    light_popup->setFixedSize(Eigen::Vector2i(120, 20));

    nanogui::Popup *light_popup_page = light_popup->popup();
    light_popup_page->setFixedWidth(230);
    nanogui::GridLayout *light_layout = new nanogui::GridLayout(
        nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle, 15, 0);
    light_layout->setColAlignment({nanogui::Alignment::Maximum, nanogui::Alignment::Fill});
    light_popup_page->setLayout(light_layout);

    nanogui::Label *light_intensity_label = new nanogui::Label(light_popup_page, "Light Intensity", "sans");
    nanogui::IntBox<int>* tb_light_intensity = new nanogui::IntBox<int>(light_popup_page);
    tb_light_intensity->setEditable(true);
    tb_light_intensity->setFixedSize(Eigen::Vector2i(120, 20));
    tb_light_intensity->setValue(50);
    tb_light_intensity->setTheme(textbox_theme);
    tb_light_intensity->setFontSize(12);
    tb_light_intensity->setFormat("[0-9]*");

    tb_light_intensity->setSpinnable(true);
    tb_light_intensity->setMinMaxValues(1, 100);
    tb_light_intensity->setValueIncrement(1);

    light_intensity = new Row(light_intensity_label, tb_light_intensity);
    light_intensity->setVisible(true);

    auto create_position_box = [this, light_popup_page](const std::string &axis) {
        new nanogui::Label(light_popup_page, axis, "sans");
        nanogui::FloatBox<float> *box = new nanogui::FloatBox<float>(light_popup_page);
        box->setEditable(true);
        box->setSpinnable(true);
        box->setValueIncrement(0.1f);
        box->setFixedSize(Eigen::Vector2i(120, 20));
        box->setTheme(textbox_theme);
        box->setFontSize(12);
        return box;
    };

    light_position_x = create_position_box("Position X");
    light_position_y = create_position_box("Position Y");
    light_position_z = create_position_box("Position Z");

    lab = new nanogui::Label(nanoguiWindow, "Shineness", "sans");
    tb = new nanogui::IntBox<int>(nanoguiWindow);
    tb->setEditable(true);
    tb->setFixedSize(Eigen::Vector2i(120, 20));
    tb->setValue(100);
    tb->setTheme(textbox_theme);
    tb->setFontSize(12);
    tb->setFormat("[0-9]*");

    tb->setSpinnable(true);
    tb->setMinValue(1);
    tb->setValueIncrement(1);

    shineness = new Row(lab, tb);
    shineness->setVisible(true);

    lab = new nanogui::Label(nanoguiWindow, "Diffuse", "sans");
    cc = new ColorController(nanoguiWindow, nanogui::Color(110,100,160, 255));
    lab = new nanogui::Label(nanoguiWindow, "Specular", "sans");
    cc2 = new ColorController(nanoguiWindow, nanogui::Color(100,100,100, 255));
    lab = new nanogui::Label(nanoguiWindow, "Ambient", "sans");
    cc3 = new ColorController(nanoguiWindow, nanogui::Color(40,40,40, 255));
    ambient = new Row(lab, cc3);
    ambient->setVisible(true);

    lab = new nanogui::Label(nanoguiWindow, "Glossiness", "sans");
    nanogui::IntBox<int>* tb2 = new nanogui::IntBox<int>(nanoguiWindow);
    tb2->setEditable(true);
    tb2->setFixedSize(Eigen::Vector2i(120, 20));
    tb2->setValue(60);
    tb2->setTheme(textbox_theme);
    tb2->setFontSize(12);
    tb2->setFormat("[0-9]*");

    tb2->setSpinnable(true);
    tb2->setMinMaxValues(1, 99);
    tb2->setValueIncrement(1);
    
    glossiness = new Row(lab, tb2);
    glossiness->setVisible(false);

    lab = new nanogui::Label(nanoguiWindow, "Ambient", "sans");
    cc4 = new ColorController(nanoguiWindow, nanogui::Color(20, 20, 20, 255));
    pbr_ambient = new Row(lab, cc4);
    pbr_ambient->setVisible(false);

    screen->setVisible(true);
    screen->performLayout();
}

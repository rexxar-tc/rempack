//
// Created by brant on 7/3/25.
//

#include "package_info_panel.h"

namespace widgets {
    void PackageInfoPanel::on_reflow() {
        _text->set_coords(x + padding, y + padding, w - (2 * padding), h - (2 * padding) - controlHeight);
        _text->mark_redraw();
        layout_buttons();
    }

    void PackageInfoPanel::set_text(string text) {
        _text->undraw();
        _text->text = std::move(text);
        _text->mark_redraw();
        this->mark_redraw();
    }

    void PackageInfoPanel::layout_image() {
        float aspect = _pixmap->icon.image.h / _pixmap->icon.image.w;
        auto nh = this->h - padding - padding;
        int nw = (int)(nh * aspect);
        _pixmap->h = nh;
        _pixmap->w = nw;
        _pixmap->x = this->x + padding;
    }

    void PackageInfoPanel::set_image(const shared_ptr<package>& package) {

        if(_pixmap == nullptr)
            _pixmap = make_shared<ui::Pixmap>(0,0,0,0,icons::Icon());
    }

    void PackageInfoPanel::set_states(bool installed, bool canPreview, bool showDownload) {
        if (installed) {
            _installBtn->disable();
            _removeBtn->enable();
        } else {
            _installBtn->enable();
            _removeBtn->disable();
        }
        if (canPreview)
            _previewBtn->show();
        else
            _previewBtn->hide();
        if (showDownload)
            _downloadBtn->show();
        else
            _downloadBtn->hide();
    }

    void PackageInfoPanel::set_actions(int add, int remove) {
        stringstream ss;
        ss << "Pending: [+" << add << "/-" << remove << "]";
        _actionCounter->textWidget->text = ss.str();
        _actionCounter->text = ss.str();
        _actionCounter->mark_redraw();
        _actionCounter->textWidget->mark_redraw();
        mark_redraw();
    }

    void PackageInfoPanel::layout_buttons() {
        auto dx = x + padding;
        auto dy = y + h - padding - controlHeight;
        _installBtn->set_coords(dx, dy, controlWidth, controlHeight);
        dx += controlWidth + padding;
        _removeBtn->set_coords(dx, dy, controlWidth, controlHeight);
        dx += controlWidth + padding;
        _downloadBtn->set_coords(dx, dy, controlWidth, controlHeight);
        dx += controlWidth + padding;
        _previewBtn->set_coords(dx, dy, controlWidth, controlHeight);

        _actionCounter->set_coords(w - controlWidth - padding, dy, controlWidth, controlHeight);

        _installBtn->on_reflow();
        _removeBtn->on_reflow();
        _downloadBtn->on_reflow();
        _previewBtn->on_reflow();
        _actionCounter->on_reflow();
        _installBtn->mark_redraw();
        _removeBtn->mark_redraw();
        _downloadBtn->mark_redraw();
        _previewBtn->mark_redraw();
        _actionCounter->mark_redraw();
    }
} // widgets

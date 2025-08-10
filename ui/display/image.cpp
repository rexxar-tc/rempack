//
// Created by brant on 7/29/25.
//

#include "image.h"

widgets::image::image(int x, int y, int w, int h){

}

void widgets::image::render() {
   // DebuggableWidget::render();
}

void widgets::image::on_reflow() {
  //  Rect::on_reflow();
}

tuple<int, int> widgets::image::get_render_size() {
  // if (this->icon.image.buffer != NULL) {
     //   return make_tuple(this->icon.image.w,  this->icon.image.h); }

    return make_tuple(0,  0);
}

void widgets::image::resize(int w, int h) {

}

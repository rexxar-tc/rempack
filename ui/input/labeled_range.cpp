//
// Created by brant on 7/3/25.
//

#include "labeled_range.h"

namespace widgets{
    void LabeledRangeInput::mark_redraw() {
        range->mark_redraw();
        if(label != nullptr)
            label->mark_redraw();
    }
}
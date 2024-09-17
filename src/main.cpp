
#include <UI.h>
#include "Rempack.h"

using namespace rmlib;

int
main(int argc, char* argv[]) {
    unistdpp::fatalOnError(runApp(Rempack()));

    auto fb = fb::FrameBuffer::open();
    if (fb.has_value()) {
        fb->canvas.set(white);
        fb->doUpdate(
          fb->canvas.rect(), fb::Waveform::GC16Fast, fb::UpdateFlags::None);
    }

    return EXIT_SUCCESS;
}

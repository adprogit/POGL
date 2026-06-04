#pragma once
#include "img/image.hh"
#include "img/image_io.hh"
#include <vector>
#include <cmath>

namespace tifo {
    rgb24_image* generate_toon_ramp_from_color(uint8_t r, uint8_t g, uint8_t b, int levels);
    rgb24_image* generate_toon_ramp(rgb24_image* texture, int levels);
}
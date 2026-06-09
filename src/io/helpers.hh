#pragma once
#include <cmath>
#include <vector>

#include "img/image.hh"
#include "img/image_io.hh"

namespace tifo
{
    rgb24_image* generate_toon_ramp_from_color(uint8_t r, uint8_t g, uint8_t b,
                                               int levels);
    rgb24_image* generate_toon_ramp(rgb24_image* texture, int levels);
} // namespace tifo
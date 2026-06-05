

#include "image.hh"

#include <cstdlib>

namespace tifo
{

    gray8_image::gray8_image(int _sx, int _sy)
    {
        sx = _sx;
        sy = _sy;

        length = sx * sy;
        // aligned_alloc requires the size to be a multiple of the alignment
        // (strictly enforced on macOS, where it otherwise returns nullptr),
        // so round the allocation up. length keeps the true data size.
        std::size_t alloc_size = (length + TL_IMAGE_ALIGNMENT - 1)
            / TL_IMAGE_ALIGNMENT * TL_IMAGE_ALIGNMENT;
        pixels = static_cast<GRAY8>(
            std::aligned_alloc(TL_IMAGE_ALIGNMENT, alloc_size));
    }

    gray8_image::~gray8_image()
    {
        std::free(pixels);
    }

    const GRAY8& gray8_image::get_buffer() const
    {
        return pixels;
    }

    GRAY8& gray8_image::get_buffer()
    {
        return pixels;
    }

    rgb24_image::rgb24_image(int _sx, int _sy)
    {
        sx = _sx;
        sy = _sy;

        length = sx * sy * 3;
        // aligned_alloc requires the size to be a multiple of the alignment
        // (strictly enforced on macOS, where it otherwise returns nullptr),
        // so round the allocation up. length keeps the true data size.
        std::size_t alloc_size = (length + TL_IMAGE_ALIGNMENT - 1)
            / TL_IMAGE_ALIGNMENT * TL_IMAGE_ALIGNMENT;
        pixels = static_cast<RGB8>(
            std::aligned_alloc(TL_IMAGE_ALIGNMENT, alloc_size));
    }

    rgb24_image::~rgb24_image()
    {
        std::free(pixels);
    }

    const RGB8& rgb24_image::get_buffer() const
    {
        return pixels;
    }

    RGB8& rgb24_image::get_buffer()
    {
        return pixels;
    }

} // namespace tifo

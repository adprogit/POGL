

#include "image_io.hh"

#include <cstdio>
#include <fstream>
#include <iostream>

namespace tifo
{

    struct struct_tga_header
    {
        uint8_t idl_length;
        uint8_t color_map_type;
        uint8_t image_type;
        uint16_t cmap_start;
        uint16_t cmap_length;
        uint8_t cmap_depth;
        uint16_t x_offset;
        uint16_t y_offset;
        uint16_t width;
        uint16_t height;
        uint8_t pixel_depth;
        uint8_t image_descriptor_alpha_channel_bits : 4;
        uint8_t image_descriptor_image_origin : 2;
        uint8_t image_descriptor_unused : 2;
    } __attribute__((packed));

    typedef struct struct_tga_header tga_header;

    tga_header new_tga_header(int width, int height)
    {
        tga_header header;
        header.idl_length = 0;
        header.color_map_type = 0;
        header.image_type = 2;
        header.cmap_start = 0;
        header.cmap_length = 0;
        header.cmap_depth = 0;
        header.x_offset = 0;
        header.y_offset = 0;
        header.width = width;
        header.height = height;
        header.pixel_depth = 24;
        header.image_descriptor_unused = 0;
        header.image_descriptor_image_origin = 0;
        header.image_descriptor_alpha_channel_bits = 0;
        return header;
    }

    bool save_image(rgb24_image& image, const char* filename)
    {
        tga_header header = new_tga_header(image.sx, image.sy);
        uint8_t* buffer_bgr;
        // FILE *f = fopen(filename, "w");
        std::ofstream outfile(filename, std::ofstream::binary);
        // if (f==0) {
        if (!outfile.is_open())
        {
            std::cerr << "ERROR: can not open " << filename
                      << " for writing!\n";
            return false;
        }

        outfile.write((char*)(&header), sizeof(tga_header));
        // fwrite(&header, sizeof(tga_header), 1, f);

        buffer_bgr = new uint8_t[image.length];
        for (std::size_t i = 0; i < image.length; i += 3)
        { // rgb2bgr
            buffer_bgr[i] = image.pixels[i + 2];
            buffer_bgr[i + 1] = image.pixels[i + 1];
            buffer_bgr[i + 2] = image.pixels[i];
        }
        // fwrite(buffer_bgr, 1, image.length, f);
        outfile.write((char*)(buffer_bgr), image.length);
        delete[] buffer_bgr;

        // fclose(f);
        outfile.close();
        return true;
    }

    rgb24_image* load_image(const char* filename)
    {
        tga_header header;
        rgb24_image* image;
        uint8_t* buffer_bgr;
        // FILE *f = fopen(filename, "r");
        std::ifstream input(filename, std::ofstream::binary);
        // if (f==0) {
        if (!input.is_open())
        {
            std::cerr << "ERROR: can not open " << filename
                      << " for reading!\n";
            return 0;
        }

        input.read((char*)&header, sizeof(tga_header));
        if (!input)
        {
            // if (fread(&header, sizeof(tga_header), 1, f)!=1) {
            std::cerr << "ERROR: can not read " << filename << "!\n";
            input.close();
            return 0;
        }

        if (header.pixel_depth != 24)
        {
            std::cerr << "ERROR: Wrong image format (not 24bits)!\n";
            input.close();
            return 0;
        }

        image = new rgb24_image(header.width, header.height);

        buffer_bgr = new uint8_t[image->length];
        input.read((char*)buffer_bgr, image->length);
        if (!input)
        {
            // if (fread(buffer_bgr, 1, image->length,
            // f)!=(unsigned)image->length) {
            std::cerr << "ERROR: can not read image data!\n";
            delete image;
            input.close();
            return 0;
        }
        for (std::size_t i = 0; i < image->length; i += 3)
        { // bgr2rgb
            image->pixels[i] = buffer_bgr[i + 2];
            image->pixels[i + 1] = buffer_bgr[i + 1];
            image->pixels[i + 2] = buffer_bgr[i];
        }
        delete[] buffer_bgr;

        // fclose(f);
        input.close();
        return image;
    }

} // namespace tifo

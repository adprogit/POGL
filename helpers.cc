
#include "helpers.hh"


namespace tifo
{
    typedef struct
    {
        unsigned int histogram[IMAGE_NB_LEVELS];
    } histogram_1d;
    histogram_1d* get_histo(gray8_image& image)
    {
        histogram_1d* histo = new histogram_1d;
        GRAY8 pixels = image.get_buffer();
        int sx = image.sx;
        int sy = image.sy;
        for (std::size_t i = 0; i < 256; i++)
        {
            histo->histogram[i] = 0;
        }
        for (int i = 0; i < sx; i++)
        {
            for (int j = 0; j < sy; j++)
            {
                unsigned int level_of_gray = pixels[i + j * sx];
                histo->histogram[level_of_gray]++;
            }
        }

        return histo;
    }
    std::vector<gray8_image*>* rgb_segmentation(rgb24_image* image)
    {
        int sx = image->sx;
        int sy = image->sy;
        std::size_t length = image->length;
        std::vector<gray8_image*>* v = new std::vector<gray8_image*>;
        gray8_image* red_image = new gray8_image(sx, sy);
        gray8_image* green_image = new gray8_image(sx, sy);
        gray8_image* blue_image = new gray8_image(sx, sy);
        std::size_t k = 0;
        for (std::size_t i = 0; i < length; i += 3)
        {
            red_image->pixels[k] = image->pixels[i];
            green_image->pixels[k] = image->pixels[i + 1];
            blue_image->pixels[k++] = image->pixels[i + 2];
        }
        v->push_back(red_image);
        v->push_back(green_image);
        v->push_back(blue_image);

        return v;
    }
    rgb24_image* three_to_one(std::vector<gray8_image*> v)
    {
        rgb24_image* img = new rgb24_image(v[0]->sx, v[0]->sy);
        img->length = v[0]->sx * v[0]->sy * 3;
        std::size_t length = img->length;
        RGB8 to_store = new uint8_t[length];
        std::size_t k = 0;
        for (std::size_t i = 0; i <= length - 3; i += 3)
        {
            to_store[i] = v[0]->pixels[k];
            to_store[i + 1] = v[1]->pixels[k];
            to_store[i + 2] = v[2]->pixels[k++];
        }
        img->pixels = to_store;
        return img;
    }

    float max3(float a, float b, float c)
    {
        return a > b ? (a > c ? a : c) : (b > c ? b : c);
    }
    float min3(float a, float b, float c)
    {
        return a < b ? (a < c ? a : c) : (b < c ? b : c);
    }
    std::vector<uint8_t> convertisseur_rgb_hsv(std::vector<uint8_t> rgb)
    {
        uint8_t r = rgb[0];
        uint8_t g = rgb[1];
        uint8_t b = rgb[2];
        float r_n = (float)r / 255;
        float g_n = (float)g / 255;
        float b_n = (float)b / 255;
        float max_n = max3(r_n, g_n, b_n);
        float min_n = min3(r_n, g_n, b_n);
        float V = max_n;
        float S = 0;
        float H = 0;

        if (max_n == min_n)
        {
            goto fin_;
        }
        S = (max_n - min_n) / max_n;
        if (max_n == r_n)
        {
            if (min_n == b_n)
            {
                H = (g_n - min_n) * 60 / (max_n - min_n);
            }
            else if (min_n == g_n)
            {
                H = (max_n - b_n) * 60 / (max_n - min_n) + 300;
            }
        }
        else if (max_n == g_n)
        {
            if (min_n == b_n)
            {
                H = (max_n - r_n) * 60 / (max_n - min_n) + 60;
            }
            else if (min_n == r_n)
            {
                H = (b_n - min_n) * 60 / (max_n - min_n) + 120;
            }
        }
        else if (max_n == b_n)
        {
            if (min_n == r_n)
            {
                H = (max_n - g_n) * 60 / (max_n - min_n) + 180;
            }
            else if (min_n == g_n)
            {
                H = (r_n - min_n) * 60 / (max_n - min_n) + 240;
            }
        }

    fin_:
        uint8_t H_u = (uint8_t)(H / 360.0f * 255.0f);
        uint8_t V_u = (uint8_t)(V * 255.0f);
        uint8_t S_u = (uint8_t)(S * 255.0f);
        std::vector<uint8_t> v;
        v.push_back(H_u);
        v.push_back(S_u);
        v.push_back(V_u);
        return v;
    }

    std::vector<uint8_t> hsv_to_rgb(std::vector<uint8_t> hsv)
    {
        uint8_t H_byte = hsv[0];
        uint8_t S_byte = hsv[1];
        uint8_t V_byte = hsv[2];
        float S = S_byte / 255.0f;
        float V = V_byte / 255.0f;
        float H = H_byte * 360.0f / 255.0f;
        float R = 0, G = 0, B = 0;
        float max_n = V;
        float min_n = max_n - S * max_n;
        if (H >= 0 && H <= 60)
        {
            R = max_n;
            G = H * (max_n - min_n) / 60 + min_n;
            B = min_n;
        }
        else if (H > 60 && H <= 120)
        {
            R = -1 * ((H - 60) * (max_n - min_n) / 60 - max_n);
            G = max_n;
            B = min_n;
        }
        else if (H > 120 && H <= 180)
        {
            R = min_n;
            G = max_n;
            B = ((H - 120) * (max_n - min_n)) / 60 + min_n;
        }
        else if (H > 180 && H <= 240)
        {
            R = min_n;
            G = -1 * ((H - 180) * (max_n - min_n) / 60 - max_n);
            B = max_n;
        }
        else if (H > 240 && H <= 300)
        {
            R = ((H - 240) * (max_n - min_n) / 60 + min_n);
            G = min_n;
            B = max_n;
        }
        else if (H > 300 && H <= 360)
        {
            R = max_n;
            G = min_n;
            B = -1 * ((H - 300) * (max_n - min_n) / 60 - max_n);
        }

        return { (uint8_t)(R * 255.0f), (uint8_t)(G * 255.0f),
                 (uint8_t)(B * 255.0f) };
    }
    std::vector<gray8_image*>* hsv_segmentation(rgb24_image* image)
    {
        int sx = image->sx;
        int sy = image->sy;
        std::size_t length = image->length;
        std::vector<gray8_image*>* v = new std::vector<gray8_image*>;
        gray8_image* hue_image = new gray8_image(sx, sy);
        gray8_image* saturation_image = new gray8_image(sx, sy);
        gray8_image* value_image = new gray8_image(sx, sy);
        std::size_t k = 0;
        for (std::size_t i = 0; i < length; i += 3)
        {
            uint8_t red_pixel = image->pixels[i];
            uint8_t green_pixel = image->pixels[i + 1];
            uint8_t blue_pixel = image->pixels[i + 2];
            std::vector<uint8_t> rgb_vec;
            rgb_vec.push_back(red_pixel);
            rgb_vec.push_back(green_pixel);
            rgb_vec.push_back(blue_pixel);
            std::vector<uint8_t> hls_vec = convertisseur_rgb_hsv(rgb_vec);
            hue_image->pixels[k] = hls_vec[0];
            saturation_image->pixels[k] = hls_vec[1];
            value_image->pixels[k++] = hls_vec[2];
        }
        v->push_back(hue_image);
        v->push_back(saturation_image);
        v->push_back(value_image);

        return v;
    }

    rgb24_image* three_hsv_to_one(std::vector<gray8_image*> v)
    {
        rgb24_image* img = new rgb24_image(v[0]->sx, v[0]->sy);
        img->length = v[0]->sx * v[0]->sy * 3;
        std::size_t length = img->length;
        RGB8 to_store = new uint8_t[length];
        std::size_t k = 0;
        for (std::size_t i = 0; i <= length - 3; i += 3)
        {
            std::vector<uint8_t> v2 = { v[0]->pixels[k], v[1]->pixels[k],
                                        v[2]->pixels[k++] };
            std::vector<uint8_t> v3 = hsv_to_rgb(v2);
            to_store[i] = v3[0];
            to_store[i + 1] = v3[1];
            to_store[i + 2] = v3[2];
        }
        img->pixels = to_store;
        return img;
    }

    rgb24_image* generate_toon_ramp_from_color(uint8_t r, uint8_t g, uint8_t b,
                                               int levels)
    {
        std::vector<uint8_t> hsv = convertisseur_rgb_hsv({ r, g, b });
        uint8_t dom_h = hsv[0], dom_s = hsv[1], dom_v = hsv[2];

        rgb24_image* ramp = new rgb24_image(256, 1);
        if (levels < 2)
            levels = 2;

        for (int x = 0; x < 256; x++)
        {
            float t = std::floor((x / 255.0f) * levels) / (float)(levels - 1);
            if (t > 1.0f)
                t = 1.0f;

            float vf = dom_v * (0.4f + 0.7f * t);
            float sf = dom_s * (1.15f - 0.2f * t);
            uint8_t v = (uint8_t)std::min(255.0f, vf);
            uint8_t s = (uint8_t)std::min(255.0f, sf);

            std::vector<uint8_t> rgb = hsv_to_rgb({ dom_h, s, v });
            ramp->pixels[x * 3 + 0] = rgb[0];
            ramp->pixels[x * 3 + 1] = rgb[1];
            ramp->pixels[x * 3 + 2] = rgb[2];
        }
        return ramp;
    }
    rgb24_image* generate_toon_ramp(rgb24_image* texture, int levels)
    {
        if (!texture || texture->length == 0)
        {
            return new rgb24_image(256, 1);
        }

        std::vector<gray8_image*>* hsv = hsv_segmentation(texture);
        if (!hsv || hsv->size() < 3)
        {
            delete hsv;
            return new rgb24_image(256, 1);
        }

        gray8_image* H = hsv->at(0);
        gray8_image* S = hsv->at(1);
        gray8_image* V = hsv->at(2);

        histogram_1d* histo_h = get_histo(*H);
        int dom_h = 0;
        unsigned int best = 0;

        for (int i = 0; i < 256; i++)
        {
            if (histo_h->histogram[i] > best)
            {
                best = histo_h->histogram[i];
                dom_h = i;
            }
        }

        unsigned long long sum_s = 0, sum_v = 0;
        std::size_t n = H->length;

        for (std::size_t i = 0; i < n; i++)
        {
            sum_s += S->pixels[i];
            sum_v += V->pixels[i];
        }

        uint8_t dom_s = (n > 0) ? (uint8_t)(sum_s / n) : 0;
        uint8_t dom_v = (n > 0) ? (uint8_t)(sum_v / n) : 0;

        rgb24_image* ramp = new rgb24_image(256, 1);
        if (levels < 2)
            levels = 2;

        float inv_levels_minus_1 = 1.0f / (float)(levels - 1);

        for (int x = 0; x < 256; x++)
        {
            float t = x / 255.0f;

            float quantized_t = std::floor(t * levels) * inv_levels_minus_1;
            if (quantized_t > 1.0f)
                quantized_t = 1.0f;

            float min_shade = 0.2f;
            float current_v =
                dom_v * (min_shade + (1.0f - min_shade) * quantized_t);
            float current_s = dom_s * (1.2f - 0.2f * quantized_t);

            uint8_t s_final = (uint8_t)std::min(255.0f, current_s);
            uint8_t v_final = (uint8_t)std::min(255.0f, current_v);

            std::vector<uint8_t> hsv_pixel = { (uint8_t)dom_h, s_final,
                                               v_final };
            std::vector<uint8_t> rgb_pixel = hsv_to_rgb(hsv_pixel);

            std::size_t idx = x * 3;
            ramp->pixels[idx + 0] = rgb_pixel[0];
            ramp->pixels[idx + 1] = rgb_pixel[1];
            ramp->pixels[idx + 2] = rgb_pixel[2];
        }

        delete histo_h;
        delete H;
        delete S;
        delete V;
        delete hsv;

        return ramp;
    }


} // namespace tifo
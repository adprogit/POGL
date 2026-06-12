#pragma once

#include "matrix4.hh"

// Every real-time tunable rendering parameter, edited by the ImGui panel and
// pushed to the shaders each frame. Defaults match the previous hardcoded
// values.
struct RenderParams
{
    // Sun
    mygl::vector3 sun_dir{ 0.55f, 0.12f, 0.82f };
    mygl::vector3 sun_color{ 1.0f, 0.55f, 0.28f };

    // Sky gradient
    mygl::vector3 sky_zenith{ 0.18f, 0.20f, 0.42f };
    mygl::vector3 sky_horizon{ 0.95f, 0.45f, 0.25f };

    // Clouds
    bool clouds_enabled = true;
    float cloud_height = 50.0f;
    float cloud_scale = 0.012f;
    float cloud_speed_u = 0.035f;
    float cloud_speed_v = 0.014f;
    float cloud_cover_low = 0.38f;
    float cloud_cover_high = 0.65f;
    float cloud_opacity = 0.95f;
    mygl::vector3 cloud_shadow{ 0.28f, 0.22f, 0.40f };
    mygl::vector3 cloud_lit{ 1.0f, 0.62f, 0.42f };

    // Ground
    mygl::vector3 ground_color{ 0.35f, 0.55f, 0.20f };
    mygl::vector3 ground_ambient{ 0.045f, 0.040f, 0.05f };
    float ground_wrap = 0.5f;

    // Toon rim darkening (trunk shader)
    float rim_low = 0.6f;
    float rim_high = 0.8f;

    // Post: god rays
    bool rays_enabled = true;
    int rays_samples = 100;
    float rays_density = 0.9f;
    float rays_decay = 0.96f;
    float rays_weight = 0.5f;
    float rays_exposure = 0.25f;

    // Post: outline
    bool outline_enabled = true;
    mygl::vector3 outline_color{ 0.05f, 0.05f, 0.08f };
    float outline_thickness = 1.0f;
    float edge_low = 0.12f;
    float edge_high = 0.35f;
    float normal_low = 0.30f;
    float normal_high = 0.80f;

    // Post: color grading
    bool grading_enabled = true;
    bool vignette_enabled = true;
    mygl::vector3 vignette_color{ 0.55f, 0.35f, 0.30f };
    float gamma = 1.7f;
};

#pragma once

#include <cstdlib>
#include <vector>

struct TreeInstance
{
    float x;
    float z;
    float rot;
    float scale;
};

inline std::vector<TreeInstance> make_scatter(int count, float spread,
                                              float min_scale, float max_scale,
                                              unsigned seed = 42)
{
    std::vector<TreeInstance> items;
    srand(seed);
    for (int i = 0; i < count; i++)
    {
        TreeInstance t;
        t.x = (rand() / (float)RAND_MAX - 0.5f) * spread;
        t.z = (rand() / (float)RAND_MAX - 0.5f) * spread;
        t.rot = (rand() / (float)RAND_MAX) * 6.28f;
        t.scale =
            min_scale + (rand() / (float)RAND_MAX) * (max_scale - min_scale);
        items.push_back(t);
    }
    return items;
}

inline std::vector<TreeInstance> make_forest(int count, float spread,
                                             unsigned seed = 42)
{
    return make_scatter(count, spread, 0.8f, 1.3f, seed);
}

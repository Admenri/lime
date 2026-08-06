#include "src/tilemap.h"

#include "src/shader.h"

namespace rgssx {

namespace {

const raylib::Rectangle kAutotileSrcRegular[] = {
    {1.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {1.5f, 2.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {1.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {0.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {0.0f, 2.5f, 0.5f, 0.5f}, {1.5f, 2.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {1.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 2.5f, 0.5f, 0.5f}, {1.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f, 0.5f},
};

const raylib::Rectangle kAutotileSrcTable[] = {
    {1.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {1.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {1.5f, 2.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {1.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {0.5f, 2.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 2.5f, 0.5f, 0.5f}, {0.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 2.0f, 0.5f, 0.5f}, {1.5f, 2.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 2.5f, 0.5f, 0.5f}, {1.5f, 2.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 2.5f, 0.5f, 0.5f}, {1.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 2.5f, 0.5f, 0.5f}, {1.5f, 2.5f, 0.5f, 0.5f},
    {0.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f, 0.5f},
};

const raylib::Rectangle kAutotileSrcWall[] = {
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f, 0.5f},
    {0.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {0.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 0.5f, 0.5f, 0.5f}, {1.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {0.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.0f, 0.5f, 0.5f}, {0.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {0.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 1.0f, 0.5f, 0.5f}, {1.5f, 1.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {1.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
    {0.0f, 0.0f, 0.5f, 0.5f}, {1.5f, 0.0f, 0.5f, 0.5f},
    {0.0f, 1.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 0.5f, 0.5f},
};

const raylib::Rectangle kAutotileSrcWaterfall[] = {
    {1.0f, 0.0f, 0.5f, 1.0f}, {0.5f, 0.0f, 0.5f, 1.0f},
    {0.0f, 0.0f, 0.5f, 1.0f}, {0.5f, 0.0f, 0.5f, 1.0f},
    {1.0f, 0.0f, 0.5f, 1.0f}, {1.5f, 0.0f, 0.5f, 1.0f},
    {0.0f, 0.0f, 0.5f, 1.0f}, {1.5f, 0.0f, 0.5f, 1.0f},
};

}  // namespace

TilemapAbove::TilemapAbove(Tilemap* parent, RefPtr<Viewport> viewport)
    : ViewportChild(viewport, ZValue(200)), parent_(parent) {}

void TilemapAbove::Draw(DrawParam param) {
  parent_->DrawMapData(true /*above*/);
}

// ----------------------------------------------------------------------

Tilemap::Tilemap(RefPtr<Viewport> viewport)
    : ViewportChild(viewport, ZValue()),
      above_(std::make_unique<TilemapAbove>(this, viewport)) {
  CreateShadowSet();
}

Tilemap::~Tilemap() {
  Dispose();
}

void Tilemap::Update() {
  if (++frame_index_ >= 30 * 3 * 4)
    frame_index_ = 0;

  const uint8_t kAniIndicesRegular[3 * 4] = {0, 1, 2, 1, 0, 1,
                                             2, 1, 0, 1, 2, 1};
  const uint8_t kAniIndicesWaterfall[3 * 4] = {0, 1, 2, 0, 1, 2,
                                               0, 1, 2, 0, 1, 2};

  regular_anim_ = kAniIndicesRegular[frame_index_ / 30];
  waterfall_anim_ = kAniIndicesWaterfall[frame_index_ / 30];

  flash_timer_ = ++flash_timer_ % 32;
  flash_opacity_ = std::abs(16 - flash_timer_) * 8 + 32;
}

void Tilemap::SetBitmap(int index, RefPtr<Bitmap> bitmap) {
  bitmaps_[index] = bitmap;
}

RefPtr<Bitmap> Tilemap::GetBitmap(int index) {
  return bitmaps_[index];
}

ATTR_DEF(RefPtr<Viewport>, Viewport, Tilemap) {
  above_->Attr_Viewport(value);
  return ViewportChild::Attr_Viewport(value);
}

ATTR_DEF(bool, Visible, Tilemap) {
  above_->Attr_Visible(value);
  return Drawable::Attr_Visible(value);
}

ATTR_DEF(int, Z, Tilemap) {
  if (value.has_value())
    above_->Attr_Z(*value + 200);
  return Drawable::Attr_Z(value);
}

ATTR_DEF(RefPtr<Table>, MapData, Tilemap) {
  if (value.has_value()) {
    map_data_ = *value;
    return std::nullopt;
  } else {
    return map_data_;
  }
}

ATTR_DEF(RefPtr<Table>, FlashData, Tilemap) {
  if (value.has_value()) {
    flash_data_ = *value;
    return std::nullopt;
  } else {
    return flash_data_;
  }
}

ATTR_DEF(RefPtr<Table>, Flags, Tilemap) {
  if (value.has_value()) {
    flags_ = *value;
    return std::nullopt;
  } else {
    return flags_;
  }
}

ATTR_DEF(int, OX, Tilemap) {
  if (value.has_value()) {
    ox_ = *value;
    return std::nullopt;
  } else {
    return ox_;
  }
}

ATTR_DEF(int, OY, Tilemap) {
  if (value.has_value()) {
    oy_ = *value;
    return std::nullopt;
  } else {
    return oy_;
  }
}

void Tilemap::DisposeObject() {
  Drawable::RemoveFromList();

  above_.reset();

  raylib::UnloadTexture(shadow_texture_);
}

void Tilemap::Draw(DrawParam param) {
  UpdateViewport(param);

  DrawMapData(false /*above*/);
}

void Tilemap::CreateShadowSet() {
  std::vector<raylib::Rectangle> rects;

  raylib::Image image = {};
  image.width = 16 * tilesize_;
  image.height = tilesize_;
  image.data = raylib::MemAlloc(image.width * image.height * 4);
  image.format = raylib::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
  image.mipmaps = 1;

  std::memset(image.data, 0, image.width * image.height * 4);

  for (int32_t i = 0; i < 16; ++i) {
    int32_t offset = i * tilesize_;
    if (i & 0x1)  // Left Top
      rects.push_back(
          raylib::Rectangle(offset, 0, tilesize_ / 2, tilesize_ / 2));
    if (i & 0x2)  // Right Top
      rects.push_back(raylib::Rectangle(offset + tilesize_ / 2, 0,
                                        tilesize_ / 2, tilesize_ / 2));
    if (i & 0x4)  // Left Bottom
      rects.push_back(raylib::Rectangle(offset, tilesize_ / 2, tilesize_ / 2,
                                        tilesize_ / 2));
    if (i & 0x8)  // Right Bottom
      rects.push_back(raylib::Rectangle(offset + tilesize_ / 2, tilesize_ / 2,
                                        tilesize_ / 2, tilesize_ / 2));
  }

  for (auto& it : rects) {
    raylib::Color shadow_tint = {0, 0, 0, 128};
    raylib::ImageDrawRectangle(&image, it.x, it.y, it.width, it.height,
                               shadow_tint);
  }

  shadow_texture_ = raylib::LoadTextureFromImage(image);
  raylib::UnloadImage(image);
}

void Tilemap::UpdateViewport(DrawParam param) {
  auto viewport = Attr_Viewport().value();
  const int viewport_ox = viewport->Attr_OX().value(),
            viewport_oy = viewport->Attr_OY().value();
  const int tilemap_real_ox = ox_ + viewport_ox,
            tilemap_real_oy = oy_ + viewport_oy;
  const int viewport_width = param.scissor.width,
            viewport_height = param.scissor.height;

  // Quad parsing viewport
  raylib::Rectangle new_viewport = {};
  new_viewport.x = tilemap_real_ox / tilesize_;
  new_viewport.y = tilemap_real_oy / tilesize_ - 1;
  new_viewport.width =
      (viewport_width / tilesize_) + !!(viewport_width % tilesize_) + 1;
  new_viewport.height =
      (viewport_height / tilesize_) + !!(viewport_height % tilesize_) + 2;
  render_viewport_ = new_viewport;

  // Rendering offset
  const int display_offset_x = tilemap_real_ox % tilesize_,
            display_offset_y = tilemap_real_oy % tilesize_;
  render_offset_ = raylib::Vector2(static_cast<float>(-display_offset_x),
                                   static_cast<float>(-display_offset_y));
  render_offset_.y -= tilesize_;

  // Apply viewport origin
  render_offset_.x += viewport_ox;
  render_offset_.y += viewport_oy;
}

void Tilemap::DrawMapData(bool above) {
  auto process_quads = [&](const raylib::Texture& texture, TileQuad* quads,
                           int32_t size) {
    for (int32_t i = 0; i < size; ++i) {
      raylib::Rectangle dest_pos = quads[i].destination;
      dest_pos.x += render_offset_.x;
      dest_pos.y += render_offset_.y;
      raylib::DrawTexturePro(texture, quads[i].source, dest_pos, {}, 0,
                             raylib::WHITE);
    }
  };

  auto value_wrap = [&](int32_t value, int32_t range) {
    int32_t res = value % range;
    return res < 0 ? res + range : res;
  };

  auto get_wrap_data = [&](RefPtr<Table> t, int32_t x, int32_t y,
                           int32_t z) -> int16_t {
    if (!t)
      return 0;

    auto tile_x = xrepeat_ ? value_wrap(x, t->XSize()) : x;
    auto tile_y = yrepeat_ ? value_wrap(y, t->YSize()) : y;

    if (!xrepeat_ && (x < 0 || x >= static_cast<int32_t>(t->XSize())))
      return 0;
    if (!yrepeat_ && (y < 0 || y >= static_cast<int32_t>(t->YSize())))
      return 0;

    return t->Get(tile_x, tile_y, z);
  };

  auto get_map_flag = [&](RefPtr<Table> t, int32_t tile_id) -> int16_t {
    if (!t)
      return 0;

    if (tile_id < 0 || tile_id >= static_cast<int32_t>(t->XSize()))
      return 0;

    return t->Get(tile_id, 0, 0);
  };

  auto autotile_set_pos = [&](raylib::Rectangle& pos, int32_t i) {
    switch (i) {
      case 0:  // Left Top
        break;
      case 1:  // Right Top
        pos.x += tilesize_ / 2.0f;
        break;
      case 2:  // Left Bottom
        pos.y += tilesize_ / 2.0f;
        break;
      case 3:  // Right bottom
        pos.x += tilesize_ / 2.0f;
        pos.y += tilesize_ / 2.0f;
        break;
      case 4:  // Table's Left Bottom
        pos.y += tilesize_ * 0.75f;
        break;
      case 5:  // Table's Right Bottom
        pos.x += tilesize_ / 2.0f;
        pos.y += tilesize_ * 0.75f;
        break;
      default:
        break;
    }
  };

  auto read_autotile_common =
      [&](int32_t pattern_id, const raylib::Texture& texture,
          const raylib::Vector2& offset, int32_t x, int32_t y,
          const raylib::Rectangle* rect_src) {
        TileQuad quads[4];

        for (int32_t i = 0; i < 4; ++i) {
          auto tex_rect_raw = rect_src[pattern_id * 4 + i];
          raylib::Rectangle tex_rect = raylib::Rectangle(
              tex_rect_raw.x * tilesize_, tex_rect_raw.y * tilesize_,
              tex_rect_raw.width * tilesize_, tex_rect_raw.height * tilesize_);
          tex_rect.x += offset.x * tilesize_ + 0.5f;
          tex_rect.y += offset.y * tilesize_ + 0.5f;
          tex_rect.width -= 1.0f;
          tex_rect.height -= 1.0f;

          raylib::Rectangle pos_rect(x * tilesize_, y * tilesize_,
                                     tilesize_ / 2.0f, tilesize_ / 2.0f);
          autotile_set_pos(pos_rect, i);

          quads[i].source = tex_rect;
          quads[i].destination = pos_rect;
        }

        process_quads(texture, quads, 4);
      };

  auto read_autotile_table = [&](int32_t pattern_id,
                                 const raylib::Texture& texture,
                                 const raylib::Vector2& offset, int32_t x,
                                 int32_t y, bool occlusion) {
    TileQuad quads[6];

    for (int32_t i = 0; i < 6; ++i) {
      const raylib::Rectangle tile_src = kAutotileSrcTable[pattern_id * 6 + i];
      raylib::Rectangle tex_rect = raylib::Rectangle(
          tile_src.x * tilesize_, tile_src.y * tilesize_,
          tile_src.width * tilesize_, tile_src.height * tilesize_);
      tex_rect.x += offset.x * tilesize_ + 0.5f;
      tex_rect.y += offset.y * tilesize_ + 0.5f;
      tex_rect.width = std::max(0.0f, tex_rect.width - 1.0f);
      tex_rect.height = std::max(0.0f, tex_rect.height - 1.0f);

      raylib::Rectangle pos_rect(x * tilesize_, y * tilesize_,
                                 tile_src.width * tilesize_,
                                 tile_src.height * tilesize_);
      autotile_set_pos(pos_rect, i);

      if (occlusion && i >= 4) {
        const float table_leg = tilesize_ * 0.25f;
        tex_rect.height -= table_leg;
        pos_rect.height -= table_leg;
      }

      quads[i].source = tex_rect;
      quads[i].destination = pos_rect;
    }

    process_quads(texture, quads, 6);
  };

  auto read_autotile_waterfall = [&](int32_t pattern_id,
                                     const raylib::Texture& texture,
                                     const raylib::Vector2& offset, int32_t x,
                                     int32_t y) {
    if (pattern_id > 0x3)
      return;

    TileQuad quads[2];

    for (size_t i = 0; i < 2; ++i) {
      auto tex_rect_raw = kAutotileSrcWaterfall[pattern_id * 2 + i];
      raylib::Rectangle tex_rect = raylib::Rectangle(
          tex_rect_raw.x * tilesize_, tex_rect_raw.y * tilesize_,
          tex_rect_raw.width * tilesize_, tex_rect_raw.height * tilesize_);
      tex_rect.x += offset.x * tilesize_ + 0.5f;
      tex_rect.y += offset.y * tilesize_ + 0.5f;
      tex_rect.width -= 1;
      tex_rect.height -= 1;

      raylib::Rectangle pos_rect(x * tilesize_ + i * (tilesize_ / 2.0f),
                                 y * tilesize_, tilesize_ / 2.0f, tilesize_);

      quads[i].source = tex_rect;
      quads[i].destination = pos_rect;
    }

    process_quads(texture, quads, 2);
  };

  auto process_tile_A1 = [&](int16_t tile_id, int32_t x, int32_t y) {
    auto& bitmap = bitmaps_[TILE_A1];
    if (bitmap && !bitmap->IsDisposed()) {
      auto& texture = bitmap->render_texture().texture;

      tile_id -= 0x0800;
      const int32_t autotile_id = tile_id / 0x30;
      const int32_t pattern_id = tile_id % 0x30;

      // clang-format off
      const raylib::Vector2 waterfall(-1, -1);
      const raylib::Vector2 src_offset[] = {
          {0,  0},  {0,  3}, // Ocean
          {6,  0},  {6,  3}, // Overlay
          {8,  0},  waterfall,
          {8,  3},  waterfall,
          {0,  6},  waterfall,
          {0,  9},  waterfall,
          {8,  6},  waterfall,
          {8,  9},  waterfall};
      const raylib::Vector2 waterfall_offset[] = {
          {14, 0}, {14, 3},
          {6,  6}, {6,  9},
          {14, 6}, {14, 9},
      };
      // clang-format on

      // Transform pattern source to waterfall style
      raylib::Vector2 src_pos = src_offset[autotile_id];
      bool waterfall_component = (src_pos.x == -1);
      bool regular_component =
          !waterfall_component && autotile_id != 2 && autotile_id != 3;

      if (waterfall_component) {
        src_pos.y += waterfall_anim_;
        read_autotile_waterfall(pattern_id, texture,
                                waterfall_offset[(autotile_id - 5) / 2], x, y);
      } else {
        if (regular_component)
          src_pos.x += 2 * regular_anim_;
        read_autotile_common(pattern_id, texture, src_pos, x, y,
                             kAutotileSrcRegular);
      }
    }
  };

  auto process_tile_A2 = [&](int16_t tile_id, int32_t x, int32_t y,
                             bool is_table, bool occlusion) {
    auto& bitmap = bitmaps_[TILE_A2];
    if (bitmap && !bitmap->IsDisposed()) {
      auto& texture = bitmap->render_texture().texture;

      tile_id -= 0x0B00;
      const int32_t autotile_id = tile_id / 0x30;
      const int32_t pattern_id = tile_id % 0x30;

      // Process table foot occlusion
      raylib::Vector2 offset((autotile_id % 8) * 2, (autotile_id / 8) * 3);
      if (is_table) {
        read_autotile_table(pattern_id, texture, offset, x, y, occlusion);
      } else {
        read_autotile_common(pattern_id, texture, offset, x, y,
                             kAutotileSrcRegular);
      }
    }
  };

  auto process_tile_A3 = [&](int16_t tile_id, int32_t x, int32_t y) {
    auto& bitmap = bitmaps_[TILE_A3];
    if (bitmap && !bitmap->IsDisposed()) {
      auto& texture = bitmap->render_texture().texture;

      tile_id -= 0x1100;
      const int32_t autotile_id = tile_id / 0x30;
      const int32_t pattern_id = tile_id % 0x30;
      if (pattern_id >= 0x10)
        return;

      const raylib::Vector2 offset((autotile_id % 8) * 2,
                                   (autotile_id / 8) * 2);
      read_autotile_common(pattern_id, texture, offset, x, y, kAutotileSrcWall);
    }
  };

  auto process_tile_A4 = [&](int16_t tile_id, int32_t x, int32_t y) {
    auto& bitmap = bitmaps_[TILE_A4];
    if (bitmap && !bitmap->IsDisposed()) {
      auto& texture = bitmap->render_texture().texture;

      tile_id -= 0x1700;
      const int32_t autotile_id = tile_id / 0x30;
      const int32_t pattern_id = tile_id % 0x30;

      const int32_t vertical_offset[] = {0, 3, 5, 8, 10, 13};
      const int32_t offset_index = autotile_id / 8;
      const raylib::Vector2 offset((autotile_id % 8) * 2,
                                   vertical_offset[offset_index]);

      if (!(offset_index % 2)) {
        read_autotile_common(pattern_id, texture, offset, x, y,
                             kAutotileSrcRegular);
      } else {
        if (pattern_id >= 0x10)
          return;

        read_autotile_common(pattern_id, texture, offset, x, y,
                             kAutotileSrcWall);
      }
    }
  };

  auto process_tile_A5 = [&](int16_t tile_id, int32_t x, int32_t y) {
    auto& bitmap = bitmaps_[TILE_A5];
    if (bitmap && !bitmap->IsDisposed()) {
      auto& texture = bitmap->render_texture().texture;

      tile_id -= 0x0600;
      int32_t ox = tile_id % 0x8;
      int32_t oy = tile_id / 0x8;

      const raylib::Vector2 atlas_position(ox * tilesize_ + 0.5f,
                                           oy * tilesize_ + 0.5f);

      raylib::Rectangle tex(atlas_position.x, atlas_position.y,
                            tilesize_ - 1.0f, tilesize_ - 1.0f);
      raylib::Rectangle pos(x * tilesize_, y * tilesize_, tilesize_, tilesize_);

      TileQuad quad;
      quad.source = tex;
      quad.destination = pos;

      process_quads(texture, &quad, 1);
    }
  };

  auto process_tile_bcde = [&](int16_t tile_id, int32_t x, int32_t y) {
    int32_t tile_type = tile_id / 0x100;
    tile_id = tile_id % 0x100;

    auto& bitmap = bitmaps_[TILE_B + tile_type];
    if (bitmap && !bitmap->IsDisposed()) {
      auto& texture = bitmap->render_texture().texture;

      int32_t ox = tile_id % 0x8;
      int32_t oy = (tile_id / 0x8) % 0x10;
      int32_t ob = tile_id / (0x8 * 0x10);

      ox += (ob % 2) * 0x8;
      oy += (ob / 2) * 0x10;

      const raylib::Vector2 atlas_position(ox * tilesize_ + 0.5f,
                                           oy * tilesize_ + 0.5f);

      raylib::Rectangle tex(atlas_position.x, atlas_position.y,
                            tilesize_ - 1.0f, tilesize_ - 1.0f);
      raylib::Rectangle pos(x * tilesize_, y * tilesize_, tilesize_, tilesize_);

      TileQuad quad;
      quad.source = tex;
      quad.destination = pos;

      process_quads(texture, &quad, 1);
    }
  };

  auto process_shadow_tile = [&](int8_t shadow_id, int32_t x, int32_t y) {
    int32_t ox = shadow_id;

    const raylib::Vector2 atlas_position(ox * tilesize_ + 0.5f,
                                         tilesize_ + 0.5f);

    raylib::Rectangle tex(atlas_position.x, atlas_position.y, tilesize_ - 1.0f,
                          tilesize_ - 1.0f);
    raylib::Rectangle pos(x * tilesize_, y * tilesize_, tilesize_, tilesize_);

    TileQuad quad;
    quad.source = tex;
    quad.destination = pos;

    process_quads(shadow_texture_, &quad, 1);
  };

  auto process_common_tile = [&](int16_t tile_id, int32_t x, int32_t y,
                                 int32_t z, int16_t under_tile_id) {
    int16_t flag = get_map_flag(flags_, tile_id);
    bool over_player = (flag & 0x10) && (z >= 2);
    bool is_table = rgss3_style_
                        ? (flag & 0x80)
                        : (tile_id - 0x0B00) % (8 * 0x30) >= (7 * 0x30);

    if (over_player == above) {
      if (tile_id >= 0x0800 && tile_id < 0x0B00)  // A1
        return process_tile_A1(tile_id, x, y);
      if (tile_id >= 0x0B00 && tile_id < 0x1100)  // A2
        return process_tile_A2(
            tile_id, x, y, is_table,
            under_tile_id >= 0x1100 && under_tile_id < 0x2000);
      if (tile_id >= 0x1100 && tile_id < 0x1700)  // A3
        return process_tile_A3(tile_id, x, y);
      if (tile_id >= 0x1700 && tile_id < 0x2000)  // A4
        return process_tile_A4(tile_id, x, y);
      if (tile_id >= 0x0600 && tile_id < 0x0680)  // A5
        return process_tile_A5(tile_id, x, y);
      if (tile_id < 0x0400)  // B ~ E
        return process_tile_bcde(tile_id, x, y);
    }
  };

  auto process_shadow_layer = [&](int32_t ox, int32_t oy, int32_t w,
                                  int32_t h) {
    if (rgss3_style_) {
      // Get shadow data from map_data[z=3] on RGSS3
      for (int32_t y = 0; y < h; ++y) {
        for (int32_t x = 0; x < w; ++x) {
          int16_t shadow_id = get_wrap_data(map_data_, x + ox, y + oy, 3);
          process_shadow_tile(shadow_id, x, y);
        }
      }
    } else {
      // Calculate shadow region on RGSS2
      for (int32_t y = 0; y < h; ++y) {
        for (int32_t x = 0; x < w; ++x) {
          if ((x + ox) % map_data_->XSize() == 0 ||
              (y + oy) % map_data_->YSize() == 0)
            continue;

          const int16_t wall_top =
              get_wrap_data(map_data_, x + ox - 1, y + oy - 1, 0);
          const int16_t wall_bottom =
              get_wrap_data(map_data_, x + ox - 1, y + oy, 0);
          const int16_t current_tile =
              get_wrap_data(map_data_, x + ox, y + oy, 0);

          const bool shadow_floor =
              (current_tile >= 0x0B00 && current_tile < 0x1100) ||
              (current_tile >= 0x0600 && current_tile < 0x0680);

          // Draw shadow if wall in A2, A5 region
          if ((wall_top >= 0x1100 && wall_top < 0x2000) &&
              (wall_bottom >= 0x1100 && wall_bottom < 0x2000) && shadow_floor) {
            // Fixed left shadow on RGSS2
            process_shadow_tile(0x05, x, y);
          }
        }
      }
    }
  };

  auto process_common_layer = [&](int32_t ox, int32_t oy, int32_t w, int32_t h,
                                  int32_t z) {
    for (int32_t y = h - 1; y >= 0; --y) {
      for (int32_t x = 0; x < w; ++x) {
        // Common tile id
        const int16_t tile_id = get_wrap_data(map_data_, x + ox, y + oy, z);
        if (!tile_id)
          continue;

        // For table foot occlusion
        const int16_t under_tile_id =
            get_wrap_data(map_data_, x + ox, y + oy + 1, 0);

        // Process tile (non-shadow tile)
        process_common_tile(tile_id, x, y, z, under_tile_id);
      }
    }
  };

  auto read_tilemap = [&](const raylib::Rectangle& viewport) {
    int32_t ox = viewport.x, oy = viewport.y;
    int32_t w = viewport.width, h = viewport.height;

    // A aera (0 - 1)
    process_common_layer(ox, oy, w, h, 0);
    process_common_layer(ox, oy, w, h, 1);

    // Shadow area (3)
    if (!above)
      process_shadow_layer(ox, oy, w, h);

    // BCDE area (2)
    process_common_layer(ox, oy, w, h, 2);
  };

  // Process tilemap data
  read_tilemap(render_viewport_);
}

}  // namespace rgssx

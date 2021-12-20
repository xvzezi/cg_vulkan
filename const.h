#pragma once


#ifndef XZ_CONST_H
#define XZ_CONST_H

#include <vector>

const uint32_t APP_WIDTH = 800;
const uint32_t APP_HEIGHT = 600;

static const char* APP_NAME = "Base Vulkan";

extern const char* DEBUG_SEGLINE;
extern const std::vector<const char*> DEBUG_VALIDATION_LAYERS;

const float RENDER_QUEUE_PRIORITY_GRAPHICS = 1.0f;
const float RENDER_QUEUE_PRIORITY_PRESENT = 1.0f;

extern const std::vector<const char*> DEVICE_EXT_REQUIRED;


const int MAX_FRAMES_IN_FLIGHT = 2;









#endif // !XZ_CONST_H
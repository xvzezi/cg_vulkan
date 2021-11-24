#pragma once
#include <iostream>
#include <vector>

const uint32_t APP_WIDTH = 800;
const uint32_t APP_HEIGHT = 600;

const char* APP_NAME = "Base Vulkan";

const char* DEBUG_SEGLINE = "--------------------------------------------\n";
const std::vector<const char*> DEBUG_VALIDATION_LAYERS = {
	"VK_LAYER_KHRONOS_validation"
};

const float RENDER_QUEUE_PRIORITY_GRAPHICS = 1.0f;
#pragma once

#ifndef XZ_UTIL_H
#define XZ_UTIL_H

#include <optional>
#include <set>
#include <vector>

#include <vulkan/vulkan.h>


struct util_QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete();

	auto uniqueIndicesWithPriorities()->std::vector<std::pair<uint32_t, float>>;

	auto toVector()->std::vector<uint32_t>;

	auto toUniqueVector()->std::vector<uint32_t>;
};

struct util_SurfaceDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	VkSurfaceFormatKHR* chooseFormat(VkFormat f = VK_FORMAT_R8G8B8A8_SRGB, VkColorSpaceKHR cs = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);

	VkPresentModeKHR choosePresentMode(VkPresentModeKHR pm = VK_PRESENT_MODE_MAILBOX_KHR);

	VkExtent2D chooseExtent(uint32_t pixel_width, uint32_t pixel_height);
};

#endif // !XZ_UTIL_H
#include "util.h"

#include "const.h"

#include <algorithm>
#include <iostream>
#include <fstream>

///// util_QueueFamilyIndices
bool util_QueueFamilyIndices::isComplete()
{
	return graphicsFamily.has_value()
		&& presentFamily.has_value();
}

auto util_QueueFamilyIndices::uniqueIndicesWithPriorities()->std::vector<std::pair<uint32_t, float>>
{
	std::set<uint32_t> uniqueFamilies;
	std::vector<std::pair<uint32_t, float>> results;
	if (graphicsFamily.has_value() && !uniqueFamilies.count(graphicsFamily.value()))
	{
		uniqueFamilies.insert(graphicsFamily.value());
		results.push_back(std::make_pair(graphicsFamily.value(), RENDER_QUEUE_PRIORITY_GRAPHICS));
	}
	if (presentFamily.has_value() && !uniqueFamilies.count(presentFamily.value()))
	{
		uniqueFamilies.insert(presentFamily.value());
		results.push_back(std::make_pair(presentFamily.value(), RENDER_QUEUE_PRIORITY_PRESENT));
	}
	return results;
}

auto util_QueueFamilyIndices::toVector() -> std::vector<uint32_t>
{
	std::vector<uint32_t> result = {
		graphicsFamily.value(), presentFamily.value()
	};
	return result;
}

auto util_QueueFamilyIndices::toUniqueVector() -> std::vector<uint32_t>
{
	const auto l = std::move(toVector());
	std::set<uint32_t> sl(l.begin(), l.end());
	return std::vector<uint32_t>(sl.begin(), sl.end());
}


///// util_SurfaceDetails
VkSurfaceFormatKHR* util_SurfaceDetails::chooseFormat(VkFormat f, VkColorSpaceKHR cs)
{
	if (formats.size() == 0) return nullptr;

	for (auto& format : formats)
	{
		if (format.format == f && format.colorSpace == cs)
			return &format;
	}

	return &formats[0];
}

VkPresentModeKHR util_SurfaceDetails::choosePresentMode(VkPresentModeKHR pm)
{
	for (const auto& mode : presentModes)
	{
		if (mode == pm) return pm;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D util_SurfaceDetails::chooseExtent(uint32_t pixel_width, uint32_t pixel_height)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;
	else
	{
		VkExtent2D extent = { pixel_width, pixel_height };
		extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return extent;
	}
}



///// utilities function
std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		throw std::runtime_error("failed to open file " + filename);

	size_t filesize = (size_t)file.tellg();
	std::vector<char> buffer(filesize);

	file.seekg(0);
	file.read(buffer.data(), filesize);

	file.close();
	return buffer;
}
#pragma once

// Allow GLFW take care of the following line
// #include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "util.h"

class BaseVulkanApplication
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	void initWindow();
	void initVulkan();
	void mainLoop();
	void cleanup();

private:	// vulkan
	auto getRequiredExtensions()->std::vector<const char*>;
	bool createInstanceHeadCheck();
	bool createInstanceLayerCheck();
	void createInstance();

	void setupDebugMessenger();

	auto findQueueFamilies(VkPhysicalDevice device)->util_QueueFamilyIndices;
	bool isDeviceSuitable(VkPhysicalDevice device);
	void pickPhysicalDevice();

	void createLogicalDevice();
	
private:
	GLFWwindow* window;
	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

private:	// debug
#ifndef NDEBUG
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	
	auto makeDebugMessengerCreateInfo()->VkDebugUtilsMessengerCreateInfoEXT;

	VkDebugUtilsMessengerEXT debugMessenger;
#endif // !NDEBUG
};
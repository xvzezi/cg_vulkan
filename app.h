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

	void createSurface();

	auto findQueueFamilies(VkPhysicalDevice device)->util_QueueFamilyIndices;
	bool checkDeviceExtSup(VkPhysicalDevice device);
	auto querySurfaceDetails(VkPhysicalDevice device)->util_SurfaceDetails;
	bool isDeviceSuitable(VkPhysicalDevice device);
	void pickPhysicalDevice();

	void createLogicalDevice();

	void createSwapChain();

	void createImageViews();

	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createRenderPass();
	void createGraphicsPipeline();

	void createFramebuffers();

	void createCommandPool();

	void createCommandBuffers();

	void createSyncObjects();

private:	// runtime

	void drawFrame();
	
	void cleanupSwapChain();

	void recreateSwapChain();

	static void framebufferResizedCallback(GLFWwindow*, int w, int h);

private:
	GLFWwindow* window;
	VkInstance instance;

	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	std::vector<VkImageView> swapChainImageViews;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;

	bool framebufferResized = false;
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
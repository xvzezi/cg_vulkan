#include "app.h"

#include "const.h"
#include "ext.h"

#include <vector>

void BaseVulkanApplication::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// hint: no OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);		// hint: un-resizable window

	window = glfwCreateWindow(APP_WIDTH, APP_HEIGHT, "Vulkan", nullptr, nullptr);
}


/**************************************** Init Vulkan ***************************************/
void BaseVulkanApplication::initVulkan()
{
	createInstance(); 
	setupDebugMessenger();
	pickPhysicalDevice();
	createLogicalDevice();
}

auto BaseVulkanApplication::getRequiredExtensions()->std::vector<const char*>
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
	requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	return requiredExtensions;
}

bool BaseVulkanApplication::createInstanceHeadCheck()
{
	std::cout << DEBUG_SEGLINE;
	std::cout << "App: " << APP_NAME << std::endl;
	std::cout << "Version: 0.1.0" << std::endl;
	// 1-1. query GLFW for required vulkan ext.
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	if (extensionCount <= 0)
	{
		throw std::runtime_error("no vulkan extension supported");
	}
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	std::cout << "available extensions:\n";
	for (const auto& ext : extensions)
	{
		std::cout << '\t' << ext.extensionName << '\t' << ext.specVersion << '\n';
	}
	// 1-2. query for glfw required extensions
	const auto& glfwExtensions = getRequiredExtensions();
	auto glfwExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
	std::cout << "glfw requires " << glfwExtensionCount << ":\n";
	for (size_t i = 0; i < glfwExtensionCount; i++)
	{
		std::cout << '\t' << glfwExtensions[i] << '\n';
	}
	// 1-3. check for unsupported extensions
	std::cout << "check for extensions:\n";
	bool result = true;
	for (size_t i = 0; i < glfwExtensionCount; i++)
	{
		bool matched = false;
		for (size_t j = 0; j < extensionCount; j++)
		{
			if (std::strcmp(glfwExtensions[i], extensions[j].extensionName) == 0)
			{
				matched = true;
				break;
			}
		}
		if (!matched)
		{
			std::cout << '\t' << glfwExtensions[i] << " NA\n";
			result = false;
		}
		else
		{
			std::cout << '\t' << glfwExtensions[i] << " ¡Ì\n";
		}
	}
	return result;
}

bool BaseVulkanApplication::createInstanceLayerCheck()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	std::cout << DEBUG_SEGLINE;
	std::cout << "enable validation layers:\n";

	bool result = true;
	for (const char* layerName : DEBUG_VALIDATION_LAYERS)
	{
		bool matched = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (std::strcmp(layerProperties.layerName, layerName) == 0)
			{
				matched = true;
				break;
			}
		}
		if (!matched)
		{
			std::cout << '\t' << layerName << "\tNA\n";
			result = false;
		}
		else
		{
			std::cout << '\t' << layerName << "\t¡Ì\n";
		}
	}
	return result;
}

void BaseVulkanApplication::createInstance()
{
#ifndef NDEBUG
	if(!createInstanceHeadCheck()) throw std::runtime_error("Instance Head Check Failed!");
#endif // !NDEBUG

	// 1. prepare application info 
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = APP_NAME;
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// 2. prepare create info
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	// query for glfw required extensions
	const auto& glfwExtensions = getRequiredExtensions();
	auto glfwExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions.data();
	// validation layers
#ifndef NDEBUG
	if (!createInstanceLayerCheck())
		throw std::runtime_error("Layer Check Failed!");
	createInfo.enabledLayerCount = static_cast<uint32_t>(DEBUG_VALIDATION_LAYERS.size());
	createInfo.ppEnabledLayerNames = DEBUG_VALIDATION_LAYERS.data();
	auto debugCreateInfo = makeDebugMessengerCreateInfo();
	createInfo.pNext = &debugCreateInfo;
#else 
	createInfo.enabledLayerCount = 0;
#endif // !NDEBUG

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if (result == VK_ERROR_LAYER_NOT_PRESENT)
		throw std::runtime_error("some layers not present!");
	else if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
		throw std::runtime_error("some extensions not present!");
	else if (result != VK_SUCCESS)
		throw std::runtime_error("failed to create instance!");
}

void BaseVulkanApplication::setupDebugMessenger()
{
#ifndef NDEBUG
	auto createInfo = makeDebugMessengerCreateInfo();
	if (ext_CreateDebugUtilsMessengerEXT(instance, &createInfo, 
		nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
#endif // NDEBUG
}

auto BaseVulkanApplication::findQueueFamilies(VkPhysicalDevice device)->util_QueueFamilyIndices
{
	util_QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		const auto& queueFamily = queueFamilies[i];
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		if (indices.isComplete()) break;
	}

	return indices;
}

bool BaseVulkanApplication::isDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

#ifndef NDEBUG
	std::cout << DEBUG_SEGLINE;
	std::cout << "Device: " << deviceProperties.deviceName << std::endl;
	std::cout << "Type: " << deviceProperties.deviceType << std::endl;
	std::cout << "2D Block Shape: " << deviceProperties.sparseProperties.residencyStandard2DBlockShape << std::endl;
	std::cout << "Aligned Mip Size: " << deviceProperties.sparseProperties.residencyAlignedMipSize << std::endl;
	std::cout << "3D Block Shape: " << deviceProperties.sparseProperties.residencyStandard3DBlockShape << std::endl;
#endif 

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

#ifndef NDBUG
	std::cout << "Features:\n";
	std::cout << "\tGeomtry Shader: " << deviceFeatures.geometryShader << std::endl;
	std::cout << "\tMulti-Viewport: " << deviceFeatures.multiViewport << std::endl;
	std::cout << "\tFloat64: " << deviceFeatures.shaderFloat64 << std::endl;
	std::cout << "\tInt16: " << deviceFeatures.shaderInt16 << std::endl;
	std::cout << "\tInt64: " << deviceFeatures.shaderInt64 << std::endl;
	std::cout << "\tTessellation Shader: " << deviceFeatures.tessellationShader << std::endl;
	std::cout << "\tWide Lines: " << deviceFeatures.wideLines << std::endl;
#endif // !NDBUG

	auto indices = findQueueFamilies(device);
	return indices.isComplete();
}

void BaseVulkanApplication::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		throw std::runtime_error("failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

#ifndef NDEBUG
	std::cout << DEBUG_SEGLINE;
	std::cout << "find available physical devices " << deviceCount << ":\n";
#endif // !NDEBUG

	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("failed to find a suitable GPU");
}

void BaseVulkanApplication::createLogicalDevice()
{
	auto indices = findQueueFamilies(physicalDevice);

	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &RENDER_QUEUE_PRIORITY_GRAPHICS;

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;

#ifndef NDEBUG
	createInfo.enabledLayerCount = static_cast<uint32_t>(DEBUG_VALIDATION_LAYERS.size());
	createInfo.ppEnabledLayerNames = DEBUG_VALIDATION_LAYERS.data();
#else 
	createInfo.enabledLayerCount = 0;
#endif // !NDEBUG

	VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to create logical device!");
}


/**************************************** Init Vulkan ***************************************/
void BaseVulkanApplication::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

}

void BaseVulkanApplication::cleanup()
{
	vkDestroyDevice(device, nullptr);

#ifndef NDEBUG
	ext_DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif // !NDEBUG

	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}

/**************************************** Debug ***************************************/
#ifndef NDEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL BaseVulkanApplication::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	//if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	//	return VK_FALSE;

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT BaseVulkanApplication::makeDebugMessengerCreateInfo()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
	return createInfo;
}
#endif
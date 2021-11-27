#include "app.h"

#include "ext.h"
#include "const.h"

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
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
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

void BaseVulkanApplication::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface");
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

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport)
			indices.presentFamily = i;

		if (indices.isComplete()) break;
	}

	return indices;
}

bool BaseVulkanApplication::checkDeviceExtSup(VkPhysicalDevice device)
{
	uint32_t extCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
	std::vector<VkExtensionProperties> validExts(extCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, validExts.data());

	std::set<std::string> queryExts(DEVICE_EXT_REQUIRED.begin(), DEVICE_EXT_REQUIRED.end());

	for (const auto& ext : validExts)
	{
		queryExts.erase(ext.extensionName);
	}

#ifndef NDEBUG
	std::cout << DEBUG_SEGLINE;
	std::cout << "PhyDevice support ext " << validExts.size() << ":\n";
	for (const auto& ext_name : DEVICE_EXT_REQUIRED)
	{
		auto tag = (queryExts.count(ext_name) == 0) ? 'y' : 'n';
		std::cout << ext_name << '\t' << tag << '\n';
	}
#endif // !NDEBUG

	return queryExts.empty();
}

auto BaseVulkanApplication::querySurfaceDetails(VkPhysicalDevice device)->util_SurfaceDetails
{
	util_SurfaceDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount > 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t modeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, nullptr);
	if (modeCount > 0)
	{
		details.presentModes.resize(modeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, details.presentModes.data());
	}

	return details;
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

#ifndef NDEBUG
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
	auto extChecked = checkDeviceExtSup(device);
	auto surfaceValid = true;
	if (extChecked)
	{
		auto surfaceDetails = querySurfaceDetails(device);
		surfaceValid = !surfaceDetails.formats.empty() && !surfaceDetails.presentModes.empty();
	}
	return indices.isComplete() && extChecked && surfaceValid;
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

	auto uniqueQueueFamilies = indices.uniqueIndicesWithPriorities();
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (const auto& queueFamilyConf : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyConf.first;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queueFamilyConf.second;
		queueCreateInfos.push_back(std::move(queueCreateInfo));
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXT_REQUIRED.size());
	createInfo.ppEnabledExtensionNames = DEVICE_EXT_REQUIRED.data();

#ifndef NDEBUG
	createInfo.enabledLayerCount = static_cast<uint32_t>(DEBUG_VALIDATION_LAYERS.size());
	createInfo.ppEnabledLayerNames = DEBUG_VALIDATION_LAYERS.data();
#else 
	createInfo.enabledLayerCount = 0;
#endif // !NDEBUG

	VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to create logical device!");

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void BaseVulkanApplication::createSwapChain()
{
	auto surfaceDetails = querySurfaceDetails(physicalDevice);
	auto surfaceFormat = surfaceDetails.chooseFormat();
	auto presentMode = surfaceDetails.choosePresentMode();

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	auto extent = surfaceDetails.chooseExtent(
		static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	uint32_t imageCount = surfaceDetails.capabilities.minImageCount + 1;
	if (surfaceDetails.capabilities.maxImageCount > 0 &&
		imageCount > surfaceDetails.capabilities.maxImageCount)
		imageCount = surfaceDetails.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat->format;
	createInfo.imageColorSpace = surfaceFormat->colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	auto indices = findQueueFamilies(physicalDevice);
	auto indexList = indices.toUniqueVector();
	if (indexList.size() > 1)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = indexList.size();
		createInfo.pQueueFamilyIndices = indexList.data();
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = surfaceDetails.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	auto result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to create swap chain!");

	uint32_t realImageCount = 0;
	vkGetSwapchainImagesKHR(device, swapChain, &realImageCount, nullptr);
	swapChainImages.resize(realImageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &realImageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat->format;
	swapChainExtent = extent;

#ifndef NDEBUG
	std::cout << DEBUG_SEGLINE;
	std::cout << "Pixel Size: " << extent.width << ", " << extent.height << std::endl;
	std::cout << "Max Pixel Size: " << surfaceDetails.capabilities.maxImageExtent.width
		<< ", " << surfaceDetails.capabilities.maxImageExtent.height << std::endl;
	std::cout << "Unique Queue Types: " << indexList.size() << std::endl;
	std::cout << "Surface Format: " << surfaceFormat->format << ", " << surfaceFormat->colorSpace << std::endl;
	std::cout << "Present Mode: " << presentMode << std::endl;
	std::cout << "\toptions: ";
	for (const auto& m : surfaceDetails.presentModes)
		std::cout << m << ", ";
	std::cout << std::endl;
	std::cout << "Swap Chain Image Count: " << realImageCount << '(' << imageCount << ')' << std::endl;
#endif // !NDEBUG

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
	vkDestroySwapchainKHR(device, swapChain, nullptr);

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

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
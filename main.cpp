#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// validationLayers are the wrappers for debugging, without these, Vulkan does no error checking
const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
};

// if we are in debug, enable validation layers, otherwise disable them
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
    /*
     * This struct is used to store the queue families that are supported by the device
     */
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() {
        /*
         * This function checks if the queue families are complete
         */
        return graphicsFamily.has_value();
    }
};

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window{};
    VkInstance instance{};
    VkDevice device {};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue{};

    void initWindow() {
        /*
         * This function initializes the GLFW window
         */
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {
        /*
         * This function initializes the Vulkan library, and creates the Vulkan instance
         */
        createInstance();
        pickPhysicalDevice();
        createLogicalDevice();
    }

    void mainLoop() {
        /*
         * This function is the main loop of the application
         */
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        /*
         * This function cleans up all the resources used by the application
         */
        vkDestroyDevice(device, nullptr);

        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void createLogicalDevice() {
        /*
         * This function creates the logical device that will be used by the application
         */
        // get the queue family indices
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        // create the queueCreateInfo struct
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};

        // make sure to add VK_KHR_portability_subset to the device extensions
        const std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                "VK_KHR_portability_subset"
        };

        // create the deviceCreateInfo struct
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        // if we are in debug, enable validation layers, otherwise disable them
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

         // create the logical device
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    }


    void pickPhysicalDevice() {
        /*
         * This function picks the physical device that will be used by the application
         */
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        // if there are no devices that support Vulkan, throw an error
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        // get all the physical devices that support Vulkan
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // check if any device is suitable, set the first suitable one as the physical device
        for (const auto& device_candidate : devices) {
            if (isDeviceSuitable(device_candidate)) {
                physicalDevice = device_candidate;
                break;
            }
        }

        // if no suitable device is found, throw an error
        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    bool isDeviceSuitable(VkPhysicalDevice device_candidate) {
        /*
         * This function checks if the device is suitable for the application
         */
        QueueFamilyIndices indices = findQueueFamilies(device_candidate);
        return indices.isComplete();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device_candidate) {
        /*
         * This function finds the queue families that are supported by the device
         */
        QueueFamilyIndices indices;
        // similar pattern as all other 'get' operations in Vulkan
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device_candidate, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device_candidate, &queueFamilyCount, queueFamilies.data());

        // iterate over the queue families and check if they support graphics
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            // check if the queue family supports graphics
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            // if the device supports graphics, and the queue family is complete, break
            if (indices.isComplete()) {
                break;
            }

            i++;
        }
        return indices;
    }

    void createInstance() {
        /*
         * This function creates the Vulkan instance, which is the connection between the application and the Vulkan library
         */

        // check if the desired validation layers are available, if not, throw an error
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // create the application info, which is a struct used to specify some details about the application
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        // create the instance create info, which is a struct used to specify some details about the instance
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        createInfo.pApplicationInfo = &appInfo;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // get the required extensions from GLFW and add them to the instance create info
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> enabledExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        // add the VK_KHR_portability_enumeration extension to the list of extensions, which is required for portability on macOS
        enabledExtensions.push_back("VK_KHR_portability_enumeration");

        createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        // create the instance
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance! Error code: " + std::to_string(result));
        }
    }

    static bool checkValidationLayerSupport() {
        /*
         * This function checks if the validation layers specified in the validationLayers vector are available
         * If they are not, it returns false, otherwise it returns true
         */
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount,availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }
        return true;
    }

};

int main() {
    std::cout << "Validation layers enabled: " << enableValidationLayers << std::endl;

    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

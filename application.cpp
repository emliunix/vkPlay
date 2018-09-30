// #define GLFW_INCLUDE_NONE
// #define GLFW_EXPOSE_NATIVE_WIN32
// #include <GLFW/glfw3.h>
// #include <GLFW/glfw3native.h>
#include "application.hpp"

// GLFWwindow* window = nullptr;

// bool initWindow()
// {
//     glfwInit();
//     glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//     window = glfwCreateWindow(800, 600, "MyApplication", nullptr, nullptr);
//     if (!window)
//         return false;

//     return true;
// }

bool Application::initVulkanEZ()
{
    VkResult result = VK_SUCCESS;
    // Create the V-EZ instance.
    VezApplicationInfo appInfo = {};
	appInfo.pApplicationName = "MyApplication";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "MyEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	VezInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.pApplicationInfo = &appInfo,

    result = vezCreateInstance(&instanceCreateInfo, &mInstance);
    if (result != VK_SUCCESS)
        return false;

    // Create a surface to render to.
    // result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    // if (result != VK_SUCCESS)
    //     return false;

    // Enumerate and select the first discrete GPU physical device.
    uint32_t physicalDeviceCount;
    vezEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vezEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, physicalDevices.data());

    for (auto pd : physicalDevices)
    {
        VkPhysicalDeviceProperties properties;
        vezGetPhysicalDeviceProperties(pd, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            mPhyDevice = pd;
            break;
        }
    }

    if (mPhyDevice == VK_NULL_HANDLE)
        return false;

    // Create a surface.
    // VezSurfaceCreateInfo createInfo = {};
    // createInfo.hinstance = GetModuleHandle(nullptr);
    // createInfo.hwnd = glfwGetWin32Window(window);
    // result = vezCreateSurface(instance, &createInfo, &surface);
    // if (result != VK_SUCCESS)
    //     return false;

    // Create a logical device connection to the physical device.
    VezDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = nullptr;
    
    result = vezCreateDevice(mPhyDevice, &deviceCreateInfo, &mDevice);
    if (result != VK_SUCCESS)
        return false;

    // Create the swapchain.
    // VezSwapchainCreateInfo swapchainCreateInfo = {};
    // swapchainCreateInfo.surface = surface;
    // swapchainCreateInfo.format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    // result = vezCreateSwapchain(device, &swapchainCreateInfo, &swapchain);
    // if (result != VK_SUCCESS)
    //     return false;

    return true;
}

#ifdef _NEVER_DEFINE
void initQueue()
{
    void vezGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
void vezGetDeviceGraphicsQueue(VkDevice device, uint32_t queueIndex, VkQueue* pQueue);
void vezGetDeviceComputeQueue(VkDevice device, uint32_t queueIndex, VkQueue* pQueue);
void vezGetDeviceTransferQueue(VkDevice device, uint32_t queueIndex, VkQueue* pQueue);

VkSemaphore semaphore = VK_NULL_HANDLE;
VkFence fence = VK_NULL_HANDLE;

VezSubmitInfo submitInfo = {};
submitInfo.commandBufferCount = 1;
submitInfo.pCommandBuffers = &commandBuffer;
submitInfo.signalSemaphoreCount = 1;
submitInfo.pSignalSemaphores = &semaphore;
VkResult result = vezQueueSubmit(queue, 1, &submitInfo, &fence);

// Pass semaphore to another vkQueueSubmit call as a wait semaphore.

// Wait on fence to complete.
vezWaitForFences(device, 1, &fence, VK_TRUE, ~0ULL);
vezDestroyFence(device, fence);
}
#endif

bool Application::prepareShader()
{
    std::string glslSource = readFile("shader/simhash.comp");
    VezShaderModuleCreateInfo createInfo = {};
	createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	createInfo.codeSize = static_cast<uint32_t>(glslSource.size());
	createInfo.pGLSLSource = glslSource.c_str();
	createInfo.pEntryPoint = "main";

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VkResult result = vezCreateShaderModule(mDevice, &createInfo, &shaderModule);
    return true;
}

void handleBuffer()
{
    // VezBufferCreateInfo createInfo = {
    //     .size = (32 << 20ULL), // 32 megabytes
    //     .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    // };
    // VkBuffer buffer = VK_NULL_HANDLE;
    // VkResult result = vezCreateBuffer(device, VEZMEMORY_CPU_TO_GPU, &createInfo, &buffer);
}

void uploadData()
{
    // STAGING Buffer
    // Copy To GPU Mem
}

int Application::main(int argc, char** argv)
{
    // if (!InitGLFW())
    // {
    //     std::cout << "Failed to create GLFW window!\n";
    //     return -1;
    // }

    if (!initVulkanEZ())
    {
        std::cout << "Failed to initialize V-EZ!\n";
        return -1;
    }

    vezDestroyDevice(mDevice);
    vezDestroyInstance(mInstance);
    return 0;
}

int main(int argc, char** argv)
{
    Application app;
    return app.main(argc, argv);
}

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

    std::vector<const char*> enabledLayers{
        "VK_LAYER_LUNARG_standard_validation",
        "VK_LAYER_LUNARG_object_tracker",
    };
    VezInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = 1;
    instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();

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
    
    VkPhysicalDeviceProperties properties;
    char useDevice[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
    for (auto pd : physicalDevices)
    {
        vezGetPhysicalDeviceProperties(pd, &properties);
    //     if (properties.deviceType & (
             //VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU |
             //VK_PHYSICAL_DEVICE_TYPE_CPU))
        if (properties.deviceType & VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            mPhyDevice = pd;
            std::memcpy(useDevice, properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
        }
        // physical device limit
        uint32_t *wgCount = properties.limits.maxComputeWorkGroupCount;
        uint32_t *wgSize = properties.limits.maxComputeWorkGroupSize;
        uint32_t wgInvoc = properties.limits.maxComputeWorkGroupInvocations;
        std::cout << "Device: " << properties.deviceName << std::endl;
        std::cout << "maxComputeSharedMemorySize: " << properties.limits.maxComputeSharedMemorySize << "b" << std::endl;
        std::cout << "maxComputeWorkGroupCount: " << wgCount[0] << ", " << wgCount[1] << ", " << wgCount[2] << std::endl;
        std::cout << "maxComputeWorkGroupSize: " << wgSize[0] << ", " << wgSize[1] << ", " << wgSize[2] << std::endl;
        std::cout << "maxComputeWorkGroupInvocations: " << properties.limits.maxComputeWorkGroupInvocations << std::endl;
        infoMemory(pd);
    }
    std::cout << "Use Device: " << useDevice << std::endl;

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

inline void print_data(uint32_t sz, uint64_t *data) {
    for (uint32_t i = 0; i < sz; ++i)
    {
        std::cout.width(5);
        std::cout << i;
        std::cout << " 0x";
        std::cout.width(16);
        char prevFill = std::cout.fill('0');
        std::cout << std::hex << data[i];
        std::cout.fill(prevFill);
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

inline void print_data_count(uint32_t sz, uint64_t *data) {
    for (uint32_t i = 0; i < sz; ++i)
    {
        std::cout.width(5);
        std::cout << i << " ";
        std::cout.width(5);
        std::cout << std::dec << data[i];
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

inline void initDataWithIndex(uint32_t dataSize, uint64_t *data)
{
    for (uint32_t i = 0; i < dataSize; ++i)
    {
        data[i] = i;
    }
    // some real test data
    data[0] = 0xbde22cbd813534ef;
}

INLINE inline void calcSimHashCPU(uint64_t *data, uint32_t dataSize, uint64_t theHash)
{
    for (uint32_t i = 0; i < dataSize; ++i)
    {
       data[i] = popcnt64(data[i] ^ theHash);
    }
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

    uint32_t dataSize = 20;
    uint64_t theHash = 0x29c3211d11255404; //0x0000111100001111;
    uint64_t *data = (uint64_t *)malloc(sizeof(uint64_t) * dataSize);;
    initDataWithIndex(dataSize, data);
    std::cout << "Low part: " << std::hex << ((uint32_t *)data)[0] << " High part: " << std::hex << ((uint32_t *)data)[1] << std::endl;
    // print simHash
    std::cout << "Simhash:" << std::endl << "      0x";
    std::cout.width(16);
    char prevFill = std::cout.fill('0');
    std::cout << std::hex << theHash << std::endl;
    std::cout.fill(prevFill);
    // print data
    std::cout << "Data:" << std::endl;
    print_data(dataSize, data);
    // compute with GPU
    SimhashVK simhash(mDevice, mPhyDevice);
    simhash.init();
    simhash.execute(dataSize, data, theHash);
    simhash.destroy();
    std::cout << "Processed Data:" << std::endl;
    print_data_count(dataSize, data);
    // compute with CPU
    initDataWithIndex(dataSize, data);
    calcSimHashCPU(data, dataSize, theHash);

    std::cout << "Expected Data:" << std::endl;
    print_data_count(dataSize, data);

    // performance test
    // GPU Part
    std::cout << "Performance test (batch = 20, loop = 3000):" << std::endl;
    std::cout << "GPU:" << std::endl;
    simhash = SimhashVK(mDevice, mPhyDevice);
    simhash.init();
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 3000; ++i)
    {
        initDataWithIndex(dataSize, data);
        simhash.execute(dataSize, data, theHash);
    }
    auto stopTime = std::chrono::high_resolution_clock::now();
    double delta = std::chrono::duration_cast<std::chrono::duration<double>>(stopTime - startTime).count();
    std::cout << "Took " << delta << "s" << std::endl;
    std::cout << 3000 * 20 / delta << "qps" << std::endl;
    simhash.destroy();
    // CPU Part
    std::cout << "CPU:" << std::endl;
    startTime = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 3000; ++i)
    {
        calcSimHashCPU(data, dataSize, theHash);
    }
    std::cout << std::endl;
    stopTime = std::chrono::high_resolution_clock::now();
    delta = std::chrono::duration_cast<std::chrono::duration<double>>(stopTime - startTime).count();
    std::cout << "Took " << delta << "s" << std::endl;
    std::cout << 3000 * 20 / delta << "qps" << std::endl;

    free((void *)data);
    vezDestroyDevice(mDevice);
    vezDestroyInstance(mInstance);

    //int pause;
    //std::cin >> pause;
    return 0;
}

int main(int argc, char** argv)
{
    Application app;
    return app.main(argc, argv);
}

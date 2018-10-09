#include "utils.hpp"

std::string readFile(const char *filename)
{
    std::ifstream is(filename, std::ios::binary);
    std::stringstream ss;
    ss << is.rdbuf();
    is.close();
    return ss.str();
}

void checkResult(VkResult result)
{
    if (VK_SUCCESS != result)
    {
        std::runtime_error("VkResult Error");
        throw "VK Exception";
        std::exit(-1);
    }
}

void printVkMemoryHeapFlags(VkMemoryHeapFlags flags)
{
    if (flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        std::cout << "  VK_MEMORY_HEAP_DEVICE_LOCAL_BIT" << std::endl;
    }
    if (flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
        std::cout << "  VK_MEMORY_HEAP_MULTI_INSTANCE_BIT" << std::endl;
    }
}

void printVkMemoryPropertyFlags(VkMemoryPropertyFlags flags)
{
    if (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
        std::cout << "  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" << std::endl;
    }
    if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        std::cout << "  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT" << std::endl;
    }
    if (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        std::cout << "  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT" << std::endl;
    }
    if (flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
        std::cout << "  VK_MEMORY_PROPERTY_HOST_CACHED_BIT" << std::endl;
    }
    if (flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
        std::cout << "  VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT" << std::endl;
    }
    if (flags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
        std::cout << "  VK_MEMORY_PROPERTY_PROTECTED_BIT" << std::endl;
    }
}

void infoMemory(VkPhysicalDevice phyDevice) {
    VkPhysicalDeviceMemoryProperties props;
    vkGetPhysicalDeviceMemoryProperties(phyDevice, &props);
    std::cout << "Heap Count: " << props.memoryHeapCount << " Type Count: " << props.memoryTypeCount << std::endl;
    std::vector<VkMemoryHeap> heaps(props.memoryHeaps, props.memoryHeaps + props.memoryHeapCount);
    int i = 0;
    for (auto h : heaps)
    {
        std::cout << "Heap " << i << " Size: " << h.size << std::endl;
        printVkMemoryHeapFlags(h.flags);
        ++i;
    }
    std::vector<VkMemoryType> types(props.memoryTypes, props.memoryTypes + props.memoryTypeCount);
    i = 0;
    for (auto t : types)
    {
        std::cout << "Type " << i << " @ Heap " << t.heapIndex << std::endl;
        printVkMemoryPropertyFlags(t.propertyFlags);
        ++i;
    }
}

uint32_t findMemory(VkPhysicalDevice phyDevice, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties props;
    vkGetPhysicalDeviceMemoryProperties(phyDevice, &props);
    for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
        if (props.memoryTypes[i].propertyFlags & flags) {
            return i;
        }
    }
    return -1;
}

VkDeviceMemory allocateMemory(VkDevice device, VkPhysicalDevice phyDevice, uint32_t typeIndex, VkDeviceSize size)
{
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = size;
    allocInfo.memoryTypeIndex = typeIndex;
    VkDeviceMemory mem = VK_NULL_HANDLE;
    checkResult(vkAllocateMemory(device, &allocInfo, nullptr, &mem));
    return mem;
}

VkBuffer createBuffer(VkDevice device, VkBufferCreateFlags createFlags, VkDeviceSize size, VkBufferUsageFlags usages, VkSharingMode sharingMode, std::vector<uint32_t> queues)
{
    // create buffer
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.flags = createFlags;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usages;
    bufferCreateInfo.sharingMode = sharingMode;
    bufferCreateInfo.queueFamilyIndexCount = queues.size();
    bufferCreateInfo.pQueueFamilyIndices = queues.data();
    VkBuffer buffer = VK_NULL_HANDLE;
    checkResult(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer));
    return buffer;
}

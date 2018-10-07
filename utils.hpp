#ifndef _UTILS_H
#define _UTILS_H

#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <VEZ.h>

std::string readFile(const char *filename);
void checkResult(VkResult result);
void infoMemory(VkPhysicalDevice phyDevice);
uint32_t findMemory(VkPhysicalDevice phyDevice, VkMemoryPropertyFlags flags);
VkDeviceMemory allocateMemory(VkDevice device, VkPhysicalDevice phyDevice, uint32_t typeIndex, VkDeviceSize size);
VkBuffer createBuffer(VkDevice device, VkBufferCreateFlags createFlags, VkDeviceSize size, VkBufferUsageFlags usages, VkSharingMode sharingMode, std::vector<uint32_t> queues);

#endif /* _UTILS_H */
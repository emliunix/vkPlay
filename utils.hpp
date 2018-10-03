#ifndef _UTILS_H
#define _UTILS_H

#include <sstream>
#include <fstream>
#include <vulkan/vulkan.h>

std::string readFile(const char *filename);
void checkResult(VkResult result);

#endif /* _UTILS_H */
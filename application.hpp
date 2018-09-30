#ifndef _APPLICATION_H
#define _APPLICATION_H
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <VEZ.h>

#include "utils.hpp"

class Application
{
public:
    bool initVulkanEZ();
    bool prepareShader();
    int main(int args, char** argv);
private:
    VkInstance mInstance;
    VkPhysicalDevice mPhyDevice;
    VkDevice mDevice;
};

#endif  /* _APPLICATION_H */

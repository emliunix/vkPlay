#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <intrin.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <VEZ.h>

#include "utils.hpp"
#include "simhashvk.hpp"

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

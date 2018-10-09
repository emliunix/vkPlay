#ifndef _APPLICATION_H
#define _APPLICATION_H

//#include <intrin.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstring>
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

#ifdef _MSC_VER
#include <intrin.h>
inline uint64_t popcnt64(uint64_t i)
{
    return __popcnt64(i);
}
#else
#include <x86intrin.h>
inline uint64_t popcnt64(uint64_t i)
{
    return _popcnt64(i);
}

#endif

#endif  /* _APPLICATION_H */

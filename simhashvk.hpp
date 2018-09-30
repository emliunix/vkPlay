#ifndef _SIMHASH_VK_H
#define _SIMHASH_VK_H

#include <VEZ.h>

class SimhashVK
{
public:
    SimhashVK(VkDevice device);
    ~SimhashVK();
private:
    VkDevice mDevice;
};
#endif /* _SIMHASH_VK_H */
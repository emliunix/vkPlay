#ifndef _SIMHASH_VK_H
#define _SIMHASH_VK_H

#include <string>
#include <vector>
#include <climits>
#include <VEZ.h>

#include "utils.hpp"

class SimhashVK
{
public:
    SimhashVK(VkDevice device);
    ~SimhashVK();
    void init();
    bool execute(uint32_t sz, uint64_t *data);
private:
    VkFence upload(uint32_t sz, uint64_t *data);
    VkFence compute();
    VkFence download(uint32_t sz, uint64_t *data);
    VkCommandBuffer createCommandBuffer();
    VezPipeline createPipeline();
    VkShaderModule createShader();
    VkQueue mTransferQueue;
    VkQueue mComputeQueue;
    VkShaderModule mSimhashShader;
    VkCommandBuffer mCmdBuffer;
    VkDevice mDevice;
    VkBuffer mStagingBuffer;
    VkBuffer mGPUBuffer;
};
#endif /* _SIMHASH_VK_H */
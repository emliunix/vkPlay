#ifndef _SIMHASH_VK_H
#define _SIMHASH_VK_H

#include <string>
#include <vector>
#include <climits>
#include <VEZ.h>

#include "utils.hpp"

#define COMPUTE_LOCAL_X 100
#define COMPUTE_LOCAL_Y 1
#define COMPUTE_LOCAL_Z 1

#define COMPUTE_BUFFER_SIZE 1048576 // 1MB
#define LOCAL_GROUP_SIZE 100

class SimhashVK
{
public:
    SimhashVK(VkDevice device);
    ~SimhashVK();
    void init();
    void destroy();
    bool execute(uint32_t sz, uint64_t *data, uint64_t simhash);
private:
    void upload(uint32_t sz, uint64_t *data);
    void compute(uint32_t size, uint64_t simhash);
    void download(uint32_t sz, uint64_t *data);
    VezPipeline createPipeline();
    VkShaderModule createShader();
    VkBuffer createStagingBuffer();
    VkBuffer createGPUBuffer();
    VkQueue mTransferQueue;
    VkQueue mComputeQueue;
    VkShaderModule mSimhashShader;
    VkCommandBuffer mCmdBuffer;
    VkDevice mDevice;
    VkBuffer mStagingBuffer;
    VkBuffer mGPUBuffer;
};
#endif /* _SIMHASH_VK_H */
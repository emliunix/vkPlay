#include "simhashvk.hpp"

SimhashVK::SimhashVK(VkDevice device) : mDevice(device)
{
    // init();
}

SimhashVK::~SimhashVK()
{
    // destroy();
}

void SimhashVK::init()
{
    mSimhashShader = createShader();
    vezGetDeviceTransferQueue(mDevice, 0, &mTransferQueue);
    vezGetDeviceComputeQueue(mDevice, 0, &mComputeQueue);
    // mCmdBuffer = createCommandBuffer();
    mStagingBuffer = createStagingBuffer();
    mGPUBuffer = createGPUBuffer();
}

void SimhashVK::destroy()
{
    vezDestroyShaderModule(mDevice, mSimhashShader);
    vezDestroyBuffer(mDevice, mStagingBuffer);
	vezDestroyBuffer(mDevice, mGPUBuffer);
}

void SimhashVK::compute(uint32_t size, uint64_t simhash)
{
    VkResult result = VK_SUCCESS;
	// pipeline
	VezPipeline pipeline = createPipeline();
	// command
	VezCommandBufferAllocateInfo cmdAI = {};
	cmdAI.queue = mComputeQueue;
	cmdAI.commandBufferCount = 1;
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	result = vezAllocateCommandBuffers(mDevice, &cmdAI, &cmd);
	checkResult(result);
	// record command
	vezBeginCommandBuffer(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	vezCmdBindPipeline(pipeline);
	vezCmdBindBuffer(mGPUBuffer, 0, VK_WHOLE_SIZE, 0, 0, 0);
	vezCmdPushConstants(0, 4, (void *)&size);
	vezCmdPushConstants(4, 8, (void *)&simhash);
	vezCmdDispatch(size / LOCAL_GROUP_SIZE + 1, 1, 1);
	vezEndCommandBuffer();
	// submit
    VezSubmitInfo submit = {};
    submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
    VkFence fence = VK_NULL_HANDLE;
    result = vezQueueSubmit(mComputeQueue, 1, &submit, &fence);
    checkResult(result);
    checkResult(vezWaitForFences(mDevice, 1, &fence, VK_TRUE, ULLONG_MAX));
	// free
	vezDestroyFence(mDevice, fence);
	vezFreeCommandBuffers(mDevice, 1, &cmd);
	vezDestroyPipeline(mDevice, pipeline);
}

void SimhashVK::upload(uint32_t sz, uint64_t *data)
{
    // write to staging
    void *ptr;
    checkResult(vezMapBuffer(mDevice, mStagingBuffer, 0, VK_WHOLE_SIZE, &ptr));
    memcpy(ptr, (void *)data, sz * 8);
    vezUnmapBuffer(mDevice, mStagingBuffer);
    // upload
    VezCommandBufferAllocateInfo cmdAI = {};
    cmdAI.queue = mTransferQueue;
    cmdAI.commandBufferCount = 1;
    VkCommandBuffer cmd;
    checkResult(vezAllocateCommandBuffers(mDevice, &cmdAI, &cmd));
    checkResult(vezBeginCommandBuffer(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));
    VezBufferCopy copy = {};
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = sz * 8;
    vezCmdCopyBuffer(mStagingBuffer, mGPUBuffer, 1, &copy);
    checkResult(vezEndCommandBuffer());
    VezSubmitInfo submit = {};
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
	VkFence fence = VK_NULL_HANDLE;
	checkResult(vezQueueSubmit(mTransferQueue, 1, &submit, &fence));
    checkResult(vezWaitForFences(mDevice, 1, &fence, VK_TRUE, ULLONG_MAX));
	vezDestroyFence(mDevice, fence);
	vezFreeCommandBuffers(mDevice, 1, &cmd);
}

void SimhashVK::download(uint32_t sz, uint64_t *data)
{

    // download data
    VezCommandBufferAllocateInfo cmdAI = {};
    cmdAI.queue = mTransferQueue;
    cmdAI.commandBufferCount = 1;
    VkCommandBuffer cmd;
    checkResult(vezAllocateCommandBuffers(mDevice, &cmdAI, &cmd));
    checkResult(vezBeginCommandBuffer(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));
    VezBufferCopy copy = {};
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = sz * 8;
    vezCmdCopyBuffer(mGPUBuffer, mStagingBuffer, 1, &copy);
    checkResult(vezEndCommandBuffer());
    VezSubmitInfo submit = {};
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
	VkFence fence = VK_NULL_HANDLE;
	checkResult(vezQueueSubmit(mTransferQueue, 1, &submit, &fence));
    checkResult(vezWaitForFences(mDevice, 1, &fence, VK_TRUE, ULLONG_MAX));
	vezDestroyFence(mDevice, fence);
	vezFreeCommandBuffers(mDevice, 1, &cmd);
    // copy back
    void *ptr;
    checkResult(vezMapBuffer(mDevice, mStagingBuffer, 0, VK_WHOLE_SIZE, &ptr));
    memcpy((void *)data, ptr, sz * 8);
    vezUnmapBuffer(mDevice, mStagingBuffer);
}

bool SimhashVK::execute(uint32_t sz, uint64_t *data, uint64_t simhash)
{
    upload(sz, data);
    compute(sz, simhash);
    download(sz, data);
    return true;
}

static inline VkBuffer createBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VezMemoryFlags memFlag)
{
    VkResult result = VK_SUCCESS;
    VkBuffer buffer;
    VezBufferCreateInfo buffCreateInfo = {};
    buffCreateInfo.size = size;
    buffCreateInfo.usage = usage;
    result = vezCreateBuffer(device, memFlag, &buffCreateInfo, &buffer);
    checkResult(result);
    return buffer;
}

VkBuffer SimhashVK::createStagingBuffer()
{
    return createBuffer(mDevice, COMPUTE_BUFFER_SIZE,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        VEZ_MEMORY_CPU_TO_GPU);
}

VkBuffer SimhashVK::createGPUBuffer()
{
    return createBuffer(mDevice, COMPUTE_BUFFER_SIZE,
                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                        VEZ_MEMORY_GPU_ONLY);
}

VkShaderModule SimhashVK::createShader()
{
    std::string glslSource = readFile("shader/simhash.comp");
    VezShaderModuleCreateInfo createInfo = {};
	createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	createInfo.codeSize = static_cast<uint32_t>(glslSource.size());
	createInfo.pGLSLSource = glslSource.c_str();
	createInfo.pEntryPoint = "main";

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VkResult result = vezCreateShaderModule(mDevice, &createInfo, &shaderModule);
    checkResult(result);
    return shaderModule;
}

VezPipeline SimhashVK::createPipeline()
{
    VezPipelineShaderStageCreateInfo stage = {};
    stage.module = mSimhashShader;
    stage.pEntryPoint = nullptr;

    VezComputePipelineCreateInfo createInfo = {};
    createInfo.pStage = &stage;

    VezPipeline pipeline = VK_NULL_HANDLE;
    VkResult result = vezCreateComputePipeline(mDevice, &createInfo, &pipeline);
    checkResult(result);
    return pipeline;
}

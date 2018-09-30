#include "simhashvk.hpp"

SimhashVK::SimhashVK(VkDevice device) : mDevice(device)
{
}

SimhashVK::~SimhashVK()
{
}

void SimhashVK::init()
{
    mSimhashShader = createShader();
    vezGetDeviceTransferQueue(mDevice, 0, &mTransferQueue);
    vezGetDeviceComputeQueue(mDevice, 0, &mComputeQueue);
    mCmdBuffer = createCommandBuffer();
}

VkCommandBuffer SimhashVK::createCommandBuffer()
{
    VkResult result = VK_SUCCESS;
    VezPipeline pipeline = createPipeline();

    VezCommandBufferAllocateInfo cmdAI = {};
    cmdAI.queue = mComputeQueue;
    cmdAI.commandBufferCount = 1;
    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
    result = vezAllocateCommandBuffers(mDevice, &cmdAI, &cmdBuffer);

    vezBeginCommandBuffer(cmdBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vezCmdBindPipeline(pipeline);
    vezCmdBindBuffer(mGPUBuffer, 0, VK_WHOLE_SIZE, 0, 0, 0);
    vezCmdDispatch(1024, 1, 1);
    vezEndCommandBuffer();

    return cmdBuffer;
}

VkFence SimhashVK::compute()
{
    VkResult result = VK_SUCCESS;
    VezSubmitInfo submit = {};
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &mCmdBuffer;
    VkFence fence;
    result = vezQueueSubmit(mComputeQueue, 1, &submit, &fence);
    return fence;
}

VkFence SimhashVK::upload(uint32_t sz, uint64_t *data)
{
    return VK_NULL_HANDLE;
}

VkFence SimhashVK::download(uint32_t sz, uint64_t *data)
{
    return VK_NULL_HANDLE;
}

bool SimhashVK::execute(uint32_t sz, uint64_t *data)
{
    VkFence fence;
    fence = upload(sz, data);
    vezWaitForFences(mDevice, 1, &fence, VK_TRUE, ULLONG_MAX);
    fence = compute();
    vezWaitForFences(mDevice, 1, &fence, VK_TRUE, ULLONG_MAX);
    fence = download(sz, data);
    vezWaitForFences(mDevice, 1, &fence, VK_TRUE, ULLONG_MAX);
    return true;
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
    return pipeline;
}

#include "simhashvk.hpp"

SimhashVK::SimhashVK(VkDevice device, VkPhysicalDevice phyDevice) : mDevice(device), mPhyDevice(phyDevice)
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
    createStagingBuffer();
    createGPUBuffer();
}

void SimhashVK::destroy()
{
    vezDestroyShaderModule(mDevice, mSimhashShader);
	vkFreeMemory(mDevice, mStagingMem, nullptr);
    vkDestroyBuffer(mDevice, mStagingBuffer, nullptr);
	vkFreeMemory(mDevice, mGPUMem, nullptr);
	vkDestroyBuffer(mDevice, mGPUBuffer, nullptr);
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
	vezCmdDispatch(size, 1, 1);
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
	checkResult(vkMapMemory(mDevice, mStagingMem, 0, VK_WHOLE_SIZE, 0, &ptr));
    memcpy(ptr, (void *)data, sz * 8);
	vkUnmapMemory(mDevice, mStagingMem);
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
    checkResult(vkMapMemory(mDevice, mStagingMem, 0, VK_WHOLE_SIZE, 0, &ptr));
    memcpy((void *)data, ptr, sz * 8);
	vkUnmapMemory(mDevice, mStagingMem);
}

bool SimhashVK::execute(uint32_t sz, uint64_t *data, uint64_t simhash)
{
    upload(sz, data);
    compute(sz, simhash);
    download(sz, data);
    return true;
}

static inline VkBuffer createBuffer2(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VezMemoryFlags memFlag)
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

void SimhashVK::createStagingBuffer()
{
	uint32_t memIndex = findMemory(mPhyDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	mStagingMem = allocateMemory(mDevice, mPhyDevice, memIndex, COMPUTE_BUFFER_SIZE);
	mStagingBuffer = createBuffer(mDevice, 0, COMPUTE_BUFFER_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, {});
	vkBindBufferMemory(mDevice, mStagingBuffer, mStagingMem, 0);
}

void SimhashVK::createGPUBuffer()
{
	uint32_t memIndex = findMemory(mPhyDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	mGPUMem = allocateMemory(mDevice, mPhyDevice, memIndex, COMPUTE_BUFFER_SIZE);
	mGPUBuffer = createBuffer(mDevice, 0, COMPUTE_BUFFER_SIZE, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, {});
	vkBindBufferMemory(mDevice, mGPUBuffer, mGPUMem, 0);
}
VkBuffer SimhashVK::createGPUBuffer2()
{
	VkPhysicalDevice phyDevice;
	VkDeviceSize size;
	// create buffer
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkBuffer buffer = VK_NULL_HANDLE;
	vkCreateBuffer(mDevice, &bufferCreateInfo, nullptr, &buffer);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = size;
	VkDeviceMemory mem = VK_NULL_HANDLE;
	vkAllocateMemory(mDevice, &allocInfo, nullptr, &mem);
	return VK_NULL_HANDLE;
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

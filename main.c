#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include "shader/simhash.h"

#define VK_DEVICE_ENUM_SIZE 10

VkInstance init_vulkan() {
  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "Hello world!!!",
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "empty",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_API_VERSION_1_1,
  };
  VkInstanceCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info,
  };
  VkInstance instance;
  if (vkCreateInstance(&create_info, NULL, &instance) != VK_SUCCESS) {
    fprintf(stderr, "failed to create instance\n");
    exit(-1);
  }
  return instance;
}

VkPhysicalDevice pick_physical_device(VkInstance instance) {
  uint32_t count;
  vkEnumeratePhysicalDevices(instance, &count, NULL);
  VkPhysicalDevice devices[count];
  vkEnumeratePhysicalDevices(instance, &count, devices);
  printf("n_devices = %d\n", count);
  for (int i = 0; i < count; i++) {
    VkPhysicalDevice d = devices[i];
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(d, &props);
    printf("Device Name: %s\n", props.deviceName);
    char *device_str;
    switch (props.deviceType) {
    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      device_str = "discrete gpu";
      break;
    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      device_str = "integrated gpu";
      break;
    VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      device_str = "virtual gpu";
      break;
    VK_PHYSICAL_DEVICE_TYPE_CPU:
      device_str = "cpu";
      break;
    VK_PHYSICAL_DEVICE_TYPE_OTHER:
      device_str = "other";
      break;
    default:
      device_str = "unknown";
      break;
    }
    printf("Device Type: %s\n", device_str);
  }
  if (count == 0) {
    fprintf(stderr, "no devices found\n");
    exit(-1);
  }
  return devices[0];
}

int find_compute_queue(VkPhysicalDevice device) {
  uint32_t q_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &q_count, NULL);
  VkQueueFamilyProperties queues[q_count];
  vkGetPhysicalDeviceQueueFamilyProperties(device, &q_count, queues);
  printf("Num queues = %d\n", q_count);
  for (int i = 0; i < q_count; ++i) {
    VkQueueFamilyProperties *q = &queues[i];
    if (q->queueCount > 0 && (q->queueFlags &VK_QUEUE_COMPUTE_BIT)) {
      printf("Compute queue index = %d\n", i);
      return i;
    }
  }
  return -1;
}

VkDevice create_device(VkPhysicalDevice phyDevice, int queue_index) {
  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo queueCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = queue_index,
    .queueCount = 1,
    .pQueuePriorities = &queuePriority,
  };
  VkPhysicalDeviceFeatures deviceFeatures = {};
  VkDeviceCreateInfo deviceCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &queueCreateInfo,
    .pEnabledFeatures = &deviceFeatures,
    .enabledExtensionCount = 0,
    .enabledLayerCount = 0,
  };
  VkDevice device;
  if (vkCreateDevice(phyDevice, &deviceCreateInfo, NULL, &device) != VK_SUCCESS) {
    fprintf(stderr, "Failed to create device.\n");
    exit(-1);
  }
  return device;
}

VkShaderModule getShader(VkDevice device) {
  VkShaderModuleCreateInfo shaderModuleCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pCode = SIMHASH_SHADER,
    .codeSize = sizeof(SIMHASH_SHADER) / sizeof(*SIMHASH_SHADER),
  };
  // printf("Code size = %d\n", (int) (sizeof(SIMHASH_SHADER) / sizeof(*SIMHASH_SHADER)));
  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &shaderModule) != VK_SUCCESS) {
    fprintf(stderr, "Failed to create shader module\n");
  }
  return shaderModule;
}

VkDeviceMemory deviceMemoryAllocate(VkPhysicalDevice phyDevice, VkDevice device, uint32_t sz) {
  // find memory
  VkPhysicalDeviceMemoryProperties memProps;
  vkGetPhysicalDeviceMemoryProperties(phyDevice, &memProps);
  int memIndex = -1;
  for (int i = 0; i < memProps.memoryTypeCount; ++i) {
    VkMemoryType *memType = &memProps.memoryTypes[i];
    if (memType->propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) &&
        memProps.memoryHeaps[memType->heapIndex].size > sz) {
      memIndex = i;
      break;
    }
  }
  if (-1 == memIndex) {
    fprintf(stderr, "Insufficient device memory.\n");
    exit(-1);
  }
  // allocate
  VkMemoryAllocateInfo memoryAllocateInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = sz,
    .memoryTypeIndex = memIndex,
  };
  VkDeviceMemory memory;
  if (vkAllocateMemory(device, &memoryAllocateInfo, 0, &memory) != VK_SUCCESS) {
    fprintf(stderr, "Failed to allocate memory.\n");
    exit(-1);
  }
  return memory;
}

void deviceMemoryFill(VkDevice device, VkDeviceMemory mem, uint32_t memSz, uint32_t sz, int32_t *src) {
  int32_t *ptr;
  if (vkMapMemory(device, mem, 0, memSz, 0, (void *)&ptr) != VK_SUCCESS) {
    fprintf(stderr, "Failed to map memory.\n");
    exit(-1);
  }
  for (uint32_t i = 0; i < sz && i < memSz; ++i) {
    ptr[i] = src[i];
  }
  vkUnmapMemory(device, mem);
}

VkBuffer createBuffer(VkDevice device) {
  VkBufferCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    NULL,
    0,
    sz,
    VK_BUFFER_USAGE_
  };
  VkBuffer buf;
  if (vkCreateBuffer(device, &createInfo, NULL, &buf) != VK_SUCCESS) {
    fprintf(stderr, "Failed to create buffer.\n");
  }
  return buf;
}

void compute(VkDevice device, VkQueue queue) {
  VkShaderModule shader = getShader(device);

  VkDescriptorSetLayoutBinding bindings[] = {
    {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    }
  };
  VkDescriptorSetLayoutCreateInfo descSetLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = 1,
    .pBindings = &bindings,
  };
  VkDescriptorSetLayout descriptorSetLayout;
  if (vkCreateDescriptorSetLayout(device, &descSetLayoutCreateInfo, NULL, &descriptorSetLayout) != VK_SUCCESS) {
    fprintf(stderr, "Failed to create descriptor set layout\n");
    exti(-1);
  }
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &descriptorSetLayout,
  };
  VkPipelineLayout pipelineLayout;
  if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
    fprintf(stderr, "Failed to create pipeline layout.\n");
    exit(-1);
  }
  VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_COMPUTE_BIT,
    .module = shader,
    .pName = "simhash shader",
  };
  VkComputePipelineCreateInfo computePipelineCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
    .stage = pipelineShaderStageCreateInfo,
    .layout = pipelineLayout,
  };
  VkPipeline pipeline;
  if (vkCreateComputePipelines(device, NULL, 1, &computePipelineCreateInfo, NULL, &pipeline) != VK_SUCCESS) {
    fprintf(stderr, "Failed to create pipeline.\n");
    exit(-1);
  }

  
  vkDestroyShaderModule(device, shader, NULL);
}

void free_vulkan(VkInstance instance, VkDevice device) {
  vkDestroyDevice(device, NULL);
  vkDestroyInstance(instance, NULL);
}

int main(int argc, char **argv) {
  VkInstance instance = init_vulkan();
  VkPhysicalDevice phyDevice = pick_physical_device(instance);
  int compute_queue_index = find_compute_queue(phyDevice);
  VkDevice device = create_device(phyDevice, compute_queue_index);
  VkQueue queue;
  vkGetDeviceQueue(device, compute_queue_index, 0, &queue);
  compute(device, queue);
  free_vulkan(instance, device);
  return 0;
}

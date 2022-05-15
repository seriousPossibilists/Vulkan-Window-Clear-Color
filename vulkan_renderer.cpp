#include "vulkan_renderer.hpp"
#include "vulkan_window.hpp"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <map>

using namespace chicken;

extern chickenWindow wndClass;


   chickenRenderer::chickenRenderer()
{
    chickenRenderer::createInstance();
    chickenRenderer::createSurface();
    chickenRenderer::pickPhysicalDevice();
    chickenRenderer::createLogicalDevice();
    chickenRenderer::createSwapChain();
}

chickenRenderer::~chickenRenderer()
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    vkDestroyDevice(device, nullptr);
}

void chickenRenderer::createInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "ChickenWindow";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

        char *extensions[] = {
#ifdef WINDOWS_BUILD
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif LINUX_BUILD
#endif
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME};
        
    uint32_t count;
    const char **extensionsGLFW = glfwGetRequiredInstanceExtensions(&count);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledExtensionNames = extensionsGLFW;
    createInfo.enabledExtensionCount = count;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
        std::cout << result << "\n";
    }
    else
    {
        std::cout << "Vulkan Instance Creation Success. \n";
    }
}

void chickenRenderer::createSurface()
{
    if (glfwCreateWindowSurface(instance, wndClass.window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
    else
    {
        std::cout << "GLFW window surface created successfully. \n";
    }
}

bool chickenRenderer::pickPhysicalDevice()
{
    uint32_t graphicsIdx = -1;

    uint32_t gpuCount = 0;
    //TODO: Suballocation from Main Allocation
    VkPhysicalDevice gpus[10];
    vkEnumeratePhysicalDevices(instance, &gpuCount, 0);
    vkEnumeratePhysicalDevices(instance, &gpuCount, gpus);

    for (uint32_t i = 0; i < gpuCount; i++)
    {
        VkPhysicalDevice gpu = gpus[i];

        uint32_t queueFamilyCount = 0;
        //TODO: Suballocation from Main Allocation

        VkQueueFamilyProperties queueProps[10];
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, 0);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueProps);

        for (uint32_t j = 0; j < queueFamilyCount; j++)
        {
            if (queueProps[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                VkBool32 surfaceSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(gpu, j, surface, &surfaceSupport);

                if (surfaceSupport)
                {
                    graphicsIdx = j;
                    gpuIntel = gpu;
                    VkPhysicalDeviceProperties physicalProperties = {};

                    break;
                }
            }
        }
    }

    if (graphicsIdx < 0)
    {
        std::cout << "No GPU support found. \n";
    }
    return true;
    std::cout << "Successfully connected with physical device, \n";
}

void chickenRenderer::createLogicalDevice()
{
    float queuePriority = 1.0;

    VkDeviceQueueCreateInfo queueInfo;
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = graphicsIdx;
    queueInfo.pQueuePriorities = &queuePriority;

    std::vector<const char *> enabledExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.enabledExtensionCount = enabledExtensions.size();
    deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
    deviceInfo.pEnabledFeatures = NULL;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.queueCreateInfoCount = 1;

    if (vkCreateDevice(gpuIntel, &deviceInfo, nullptr, &device) != VK_SUCCESS)
    {
        std::cout << "Failed to create device. \n";
    }

    vkGetDeviceQueue(device, graphicsIdx, 0, &graphicsQueue);
}

void chickenRenderer::createSwapChain()
{       
    uint32_t formatCount = 0;
    VkSurfaceFormatKHR surfaceFormats[10];
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpuIntel, surface, &formatCount, 0);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpuIntel, surface, &formatCount, surfaceFormats);

    for(uint32_t i = 0; i < formatCount; i++)
    {
        VkSurfaceFormatKHR format = surfaceFormats[i];

        if(format.format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            surfaceFormat = format;
            break;
        }
    }

    VkSurfaceCapabilitiesKHR surfaceCapibilities;
    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpuIntel, surface, &surfaceCapibilities);
    uint32_t imgCount = 0;
    imgCount = surfaceCapibilities.minImageCount + 1;
    imgCount > surfaceCapibilities.maxImageCount ? imgCount - 1: imgCount;

    VkSwapchainCreateInfoKHR scInfo = {};
    scInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    scInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    scInfo.surface = surface;
    scInfo.imageFormat = surfaceFormat.format;
    scInfo.preTransform = surfaceCapibilities.currentTransform;
    scInfo.imageExtent = surfaceCapibilities.currentExtent;
    scInfo.minImageCount = imgCount;
    scInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    scInfo.imageArrayLayers = 1;

    if (vkCreateSwapchainKHR(device, &scInfo, nullptr, &swapchain) != VK_SUCCESS)
    {
        std::cout << "Failed to createSwapChain. \n";
    }
    else
    {
        std::cout << "Created swapchain successfully. \n";
    }


    vkGetSwapchainImagesKHR(device, swapchain, &scImgCount, 0);
    vkGetSwapchainImagesKHR(device, swapchain, &scImgCount, scImages);

    {
        VkAttachmentDescription attachment = {};
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.format = surfaceFormat.format;


        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        

    
        VkSubpassDescription subpassDesc = {};
        subpassDesc.colorAttachmentCount = 1;
        subpassDesc.pColorAttachments = &colorAttachmentRef;

        VkAttachmentDescription attachments[] = {
            attachment
        };

        VkRenderPassCreateInfo rpInfo = {};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpInfo.pAttachments = attachments;
        rpInfo.attachmentCount= sizeof(attachments);
        rpInfo.subpassCount = 1;
        rpInfo.pSubpasses = &subpassDesc;

        vkCreateRenderPass(device, &rpInfo, 0, &renderpass);
    }

    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = graphicsIdx;
        vkCreateCommandPool(device, &poolInfo, 0, &commandPool);
    }

    {   
        VkSemaphoreCreateInfo semainfo = {};
        semainfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(device, &semainfo, 0, &acquireSemaphore);
        vkCreateSemaphore(device, &semainfo, 0, &submitSemaphore);
    }

}

bool chickenRenderer::vk_render()
{
    uint32_t imgIdx;
    vkAcquireNextImageKHR(device, swapchain, 0, acquireSemaphore, 0,&imgIdx);

    VkCommandBuffer cmd;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = commandPool; 
    vkAllocateCommandBuffers(device, &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


    vkBeginCommandBuffer(cmd, &beginInfo);

    VkRenderPassBeginInfo rpBeginInfo = {};
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    

    vkCmdBeginRenderPass(cmd, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    {
        VkClearColorValue color = {1, 0, 0, 1};
        VkImageSubresourceRange range = {};
        range.layerCount = 1;
        range.levelCount = 1;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        vkCmdClearColorImage(cmd, scImages[imgIdx], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &color, 1, &range);

    }

    vkEndCommandBuffer(cmd);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.pSignalSemaphores = &submitSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &acquireSemaphore;
    submitInfo.waitSemaphoreCount = 1;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, 0);


    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pImageIndices  = &imgIdx;
    presentInfo.pWaitSemaphores = &submitSemaphore; 
    presentInfo.waitSemaphoreCount = 1;
    vkQueuePresentKHR(graphicsQueue, &presentInfo);

    vkFreeCommandBuffers(device, commandPool, 1, &cmd);

    return true;
}

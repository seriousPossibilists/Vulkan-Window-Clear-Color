#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace chicken {

    class chickenRenderer{
        public:
        chickenRenderer();
        ~chickenRenderer();
        bool vk_render();


        private:
        VkInstance instance;
        VkSurfaceKHR surface;
        VkPhysicalDevice physicalDevice[10];
        VkPhysicalDevice gpuIntel;
        VkDevice device;
        VkSwapchainKHR swapchain;
        VkSurfaceFormatKHR surfaceFormat;
        VkQueue graphicsQueue;
        VkCommandPool commandPool;
        VkSemaphore submitSemaphore;
        VkSemaphore acquireSemaphore;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkRenderPass renderpass;
        int graphicsIdx;

        uint32_t scImgCount;
        VkImage scImages[5] ;

        void createInstance();
        void createSurface();
        bool pickPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
    };
}
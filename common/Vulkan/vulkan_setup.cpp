

#include "vulkan_setup.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>


#include <iostream>
#include <vector>

#include <fstream>


using namespace std;


typedef PFN_vkVoidFunction(VKAPI_PTR* PFN_vkGetInstanceProcAddr)(VkInstance instance, const char* pName);



//#pragma comment(lib, "vulkan-1")




#define NUMBER_OF_XWORKGROUPS	8

// the size in bytes of the scratch space for compute shader input (start/end index,workspace synchronization init, CLUT, VARS, etc)
// this should always be the same
#define SCRATCH_SPACE_SIZE_BYTES	(16384)


static constexpr uint32_t DEFAULT_NUM_XWORKGROUPS = 1;

uint32_t vulkan_screen_size_x;
uint32_t vulkan_screen_size_y;

uint32_t vulkan_num_gpu_x_workgroups = DEFAULT_NUM_XWORKGROUPS;


uint32_t vulkan_SubgroupSize;
uint32_t vulkan_NumSubgroups;
uint32_t vulkan_WorkgroupSize;
uint32_t vulkan_NumWorkgroups;


VkInstance vulkan_instance;

VkPhysicalDevice vulkan_physicalDevice;

VkDevice vulkan_device;

VkDeviceMemory vulkan_memory;
VkDeviceMemory vulkan_memory_local;
VkDeviceMemory vulkan_memory_image;


std::vector<VkBuffer> vulkan_buffers(0);


// buffer for the scratch memory
//VkBuffer vulkan_scratch_buffer;

// buffer for the gpu memory
//VkBuffer vulkan_inout_buffer;

// buffer for the screen output
//VkBuffer vulkan_out_buffer;

// buffer for the input draw commands
//VkBuffer vulkan_incomm_buffer;

// buffer for the input graphics data
//VkBuffer vulkan_indata_buffer;

// buffer for parallel computations/rasterization
//VkBuffer vulkan_work_buffer;

// faster to write screen into a buffer than an image from compute shader ?
//VkBuffer vulkan_staging_buffer;

VkShaderModule vulkan_shader_module;

VkDescriptorSetLayout vulkan_descriptorSetLayout;

VkPipelineLayout vulkan_pipelineLayout;
VkPipeline vulkan_pipeline;

VkCommandPool vulkan_commandPool;
VkCommandBuffer vulkan_commandBuffer;
VkQueue vulkan_queue;

VkSubmitInfo vulkan_submitInfo;

uint32_t vulkan_queueFamilyIndex;

VkSemaphore vulkan_image_available_semaphore;
VkSemaphore vulkan_rendering_finished_semaphore;

VkSurfaceKHR vulkan_presentation_surface;

VkSwapchainKHR vulkan_swapchain;


// will need to resize this later
std::vector<VkCommandBuffer> vulkan_PresentQueueCmdBuffers(0);


VkDescriptorPool vulkan_descriptorPool;

// this is the image that the compute shader draws the screen into, before the blit to the program window
// now it's going to do a copy which is faster, then the blit to match the size of the screen
VkImage vulkan_image;

VkDescriptorSet vulkan_descriptorSet;


VkFence vulkan_fence;


HMODULE VulkanLibrary;

bool vulkan_load_library()
{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VulkanLibrary = LoadLibrary("vulkan-1.dll");
#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR)
	VulkanLibrary = dlopen("libvulkan.so", RTLD_NOW);
#endif

	if (VulkanLibrary == nullptr) {
		std::cout << "Could not load Vulkan library!" << std::endl;
		return false;
	}
	return true;
}


namespace vulkanapi {

#define VK_EXPORTED_FUNCTION( fun ) PFN_##fun fun;
#define VK_GLOBAL_LEVEL_FUNCTION( fun ) PFN_##fun fun;
#define VK_INSTANCE_LEVEL_FUNCTION( fun ) PFN_##fun fun;
#define VK_DEVICE_LEVEL_FUNCTION( fun ) PFN_##fun fun;

#include "vulkan_list_of_functions.inl"


	bool vulkan_load_exported_entry_points()
	{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
#define LoadProcAddress GetProcAddress
#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR)
#define LoadProcAddress dlsym
#endif

#define VK_EXPORTED_FUNCTION( fun ) \
if( !(fun = (PFN_##fun)LoadProcAddress( VulkanLibrary, #fun )) ) { \
  std::cout << "Could not load exported function: " << #fun << "!" << std::endl; \
  return false; \
}

#include "vulkan_list_of_functions.inl"

		return true;
	}


	bool vulkan_load_global_level_entry_points()
	{
#define VK_GLOBAL_LEVEL_FUNCTION( fun )                                               \
if( !(fun = (PFN_##fun)vkGetInstanceProcAddr( nullptr, #fun )) ) {                    \
  std::cout << "Could not load global level function: " << #fun << "!" << std::endl;  \
  return false;                                                                       \
}

#include "vulkan_list_of_functions.inl"

		return true;
	}


	bool vulkan_load_instance_level_entry_points()
	{
#define VK_INSTANCE_LEVEL_FUNCTION( fun )                                               \
if( !(fun = (PFN_##fun)vkGetInstanceProcAddr( vulkan_instance, #fun )) ) {              \
  std::cout << "Could not load instance level function: " << #fun << "!" << std::endl;  \
  return false;                                                                         \
}

#include "vulkan_list_of_functions.inl"

		return true;
	}


	bool vulkan_load_device_level_entry_points()
	{
#define VK_DEVICE_LEVEL_FUNCTION( fun )                                               \
if( !(fun = (PFN_##fun)vkGetDeviceProcAddr( vulkan_device, #fun )) ) {                \
  std::cout << "Could not load device level function: " << #fun << "!" << std::endl;  \
  return false;                                                                       \
}

#include "vulkan_list_of_functions.inl"

		return true;
	}

}


int vulkan_get_subgroup_size()
{
	return vulkan_SubgroupSize;
}


void vulkan_set_screen_size(int screen_size_x, int screen_size_y)
{
	vulkan_screen_size_x = (uint32_t)screen_size_x;
	vulkan_screen_size_y = (uint32_t)screen_size_y;
}


void vulkan_set_gpu_threads(int num_gpu_threads)
{
	vulkan_num_gpu_x_workgroups = (uint32_t)num_gpu_threads;
}


// wait for gpu with vulkan
bool vulkan_wait()
{
	// get device queue ??
	vulkanapi::vkGetDeviceQueue(vulkan_device, vulkan_queueFamilyIndex, 0, &vulkan_queue);

	VkResult v1 = vulkanapi::vkQueueWaitIdle(vulkan_queue);
	if ( v1 != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem with idle wait.\n";
		std::cout << "\nError value: " << v1;
		return false;
	}

	return true;
}


// wait for gpu fence with vulkan
bool vulkan_wait_fence()
{
	VkResult res = vulkanapi::vkWaitForFences(vulkan_device, 1, &vulkan_fence, VK_TRUE, UINT64_MAX);
	if (res != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem with idle FENCE wait.\n";
		std::cout << "\nError value: " << res;
		return false;
	}

	vulkanapi::vkResetFences(vulkan_device, 1, &vulkan_fence);
	return true;
}

// flush mapped memory ??
void vulkan_flush()
{
	VkMappedMemoryRange memrange = {
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		nullptr,
		vulkan_memory,
		0,
		VK_WHOLE_SIZE
	};

	if (vulkanapi::vkFlushMappedMemoryRanges(vulkan_device, 1, &memrange) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN: ERROR: problem flushing mapped memory range.\n";
	}
}


bool vulkan_execute_compute_only()
{
	//if (vkQueueWaitIdle(vulkan_queue) != VK_SUCCESS)
	//{
	//	cout << "\nERROR: VULKAN: Problem with idle wait during vulkan_execute.\n";
	//}

	//if (vulkanapi::vkQueueSubmit(vulkan_queue, 1, &vulkan_submitInfo, 0) != VK_SUCCESS)
	if (vulkanapi::vkQueueSubmit(vulkan_queue, 1, &vulkan_submitInfo, vulkan_fence) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem with queue submit (COMPUTE ONLY).\n";
		return false;
	}

	return true;
}

// submit the command to run the compute job
// can add more commands later as needed, should be same for both ps1 and ps2
bool vulkan_execute()
{
#ifdef VULKAN_EXECUTE_COMPUTE_ONLY

	// get device queue ??
	vulkanapi::vkGetDeviceQueue(vulkan_device, vulkan_queueFamilyIndex, 0, &vulkan_queue);

	if (vulkanapi::vkQueueWaitIdle(vulkan_queue) != VK_SUCCESS)
	{
		std::cout << "\nERROR: VULKAN: Problem with idle wait during vulkan_execute.\n";

	}

	if (vulkanapi::vkQueueSubmit(vulkan_queue, 1, &vulkan_submitInfo, 0) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem with queue submit.\n";
	}
#else

	//if (vkQueueWaitIdle(vulkan_queue) != VK_SUCCESS)
	//{
	//	cout << "\nERROR: VULKAN: Problem with idle wait during vulkan_execute.\n";
	//}

	uint32_t image_index;
	VkResult result = vulkanapi::vkAcquireNextImageKHR(vulkan_device, vulkan_swapchain, UINT64_MAX, vulkan_image_available_semaphore, VK_NULL_HANDLE, &image_index);
	switch (result) {
	case VK_SUCCESS:
	case VK_SUBOPTIMAL_KHR:
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		// TODO: recreate swap stuff
		//return OnWindowSizeChanged();
		std::cout << "Problem occurred during swap chain image acquisition!: VK_ERROR_OUT_OF_DATE_KHR" << std::endl;

		// try recreating swap chain ?
		vulkan_create_swap_chain();
		return true;

	default:

		std::cout << "Problem occurred during swap chain image acquisition!:" << result << std::endl;

		return false;
	}


	VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo submit_info = {
	  VK_STRUCTURE_TYPE_SUBMIT_INFO,                // VkStructureType              sType
	  nullptr,                                      // const void                  *pNext
	  1,                                            // uint32_t                     waitSemaphoreCount
	  & vulkan_image_available_semaphore,              // const VkSemaphore           *pWaitSemaphores
	  & wait_dst_stage_mask,                         // const VkPipelineStageFlags  *pWaitDstStageMask;
	  1,                                            // uint32_t                     commandBufferCount
	  & vulkan_PresentQueueCmdBuffers[image_index],  // const VkCommandBuffer       *pCommandBuffers
	  1,                                            // uint32_t                     signalSemaphoreCount
	  & vulkan_rendering_finished_semaphore            // const VkSemaphore           *pSignalSemaphores
	};

	//if (vulkanapi::vkQueueSubmit(vulkan_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
	if (vulkanapi::vkQueueSubmit(vulkan_queue, 1, &submit_info, vulkan_fence) != VK_SUCCESS) {
			//return false;
		cout << "\nVULKAN ERROR: Problem with queue submit.\n";
		return false;
	}


	VkPresentInfoKHR present_info = {
		  VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,           // VkStructureType              sType
		  nullptr,                                      // const void                  *pNext
		  1,                                            // uint32_t                     waitSemaphoreCount
		  & vulkan_rendering_finished_semaphore,           // const VkSemaphore           *pWaitSemaphores
		  1,                                            // uint32_t                     swapchainCount
		  & vulkan_swapchain,                            // const VkSwapchainKHR        *pSwapchains
		  & image_index,                                 // const uint32_t              *pImageIndices
		  nullptr                                       // VkResult                    *pResults
	};
	result = vulkanapi::vkQueuePresentKHR(vulkan_queue, &present_info);

	switch (result) {
	case VK_SUCCESS:
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
	case VK_SUBOPTIMAL_KHR:
		// TODO: recreate swap stuff
		//return OnWindowSizeChanged();
		std::cout << "Problem occurred during image presentation!: VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR" << std::endl;

		// try recreating swap chain ?
		vulkan_create_swap_chain();
		break;

	default:
		std::cout << "Problem occurred during image presentation!:" << result << std::endl;

		return false;
		break;
	}

#endif

	return true;
}



// when parameters change need to recreate presentation queue
void vulkan_clear()
{
	if (vulkan_device != VK_NULL_HANDLE)
	{
		vulkanapi::vkDeviceWaitIdle(vulkan_device);

		if (vulkan_commandPool != VK_NULL_HANDLE) {
			// free command buffer
			vulkanapi::vkFreeCommandBuffers(vulkan_device, vulkan_commandPool, 1, &vulkan_commandBuffer);

			//if ((vulkan_PresentQueueCmdBuffers.size() > 0) && (vulkan_PresentQueueCmdBuffers[0] != VK_NULL_HANDLE)) {
			if (vulkan_PresentQueueCmdBuffers.size() > 0) {
				vulkanapi::vkFreeCommandBuffers(vulkan_device, vulkan_commandPool, static_cast<uint32_t>(vulkan_PresentQueueCmdBuffers.size()), &vulkan_PresentQueueCmdBuffers[0]);
				vulkan_PresentQueueCmdBuffers.clear();
			}

			vulkanapi::vkDestroyCommandPool(vulkan_device, vulkan_commandPool, nullptr);
			vulkan_commandPool = VK_NULL_HANDLE;
		}
	}
}


void vulkan_destroy()
{
	// clear presentation queue stuff
	vulkan_clear();

	if (vulkan_device != VK_NULL_HANDLE)
	{
		vulkanapi::vkDeviceWaitIdle(vulkan_device);

		vulkanapi::vkUnmapMemory(vulkan_device, vulkan_memory);

		vulkanapi::vkDestroyFence(vulkan_device, vulkan_fence, nullptr);

		vulkanapi::vkFreeDescriptorSets(vulkan_device, vulkan_descriptorPool, 1, &vulkan_descriptorSet);
		vulkanapi::vkDestroyDescriptorPool(vulkan_device, vulkan_descriptorPool, nullptr);

		vulkanapi::vkDestroyPipeline(vulkan_device, vulkan_pipeline, nullptr);
		vulkanapi::vkDestroyPipelineLayout(vulkan_device, vulkan_pipelineLayout, nullptr);

		vulkanapi::vkDestroyDescriptorSetLayout(vulkan_device, vulkan_descriptorSetLayout, nullptr);

		vulkanapi::vkDestroyShaderModule(vulkan_device, vulkan_shader_module, nullptr);

		vulkanapi::vkDestroyImage(vulkan_device, vulkan_image, nullptr);

		for (int i = 0; i < vulkan_buffers.size(); i++)
		{
			vulkanapi::vkDestroyBuffer(vulkan_device, vulkan_buffers[i], nullptr);
		}
		vulkanapi::vkFreeMemory(vulkan_device, vulkan_memory, nullptr);
		vulkanapi::vkFreeMemory(vulkan_device, vulkan_memory_local, nullptr);
		vulkanapi::vkFreeMemory(vulkan_device, vulkan_memory_image, nullptr);

		//vulkanapi::vkDestroyBuffer(vulkan_device, vulkan_scratch_buffer, nullptr);
		//vulkanapi::vkDestroyBuffer(vulkan_device, vulkan_inout_buffer, nullptr);
		//vulkanapi::vkDestroyBuffer(vulkan_device, vulkan_out_buffer, nullptr);
		//vulkanapi::vkDestroyBuffer(vulkan_device, vulkan_incomm_buffer, nullptr);
		//vulkanapi::vkDestroyBuffer(vulkan_device, vulkan_indata_buffer, nullptr);
		//vulkanapi::vkDestroyBuffer(vulkan_device, vulkan_work_buffer, nullptr);
		//vulkanapi::vkDestroyBuffer(vulkan_device, vulkan_staging_buffer, nullptr);


		if (vulkan_image_available_semaphore != VK_NULL_HANDLE) {
			vulkanapi::vkDestroySemaphore(vulkan_device, vulkan_image_available_semaphore, nullptr);
			vulkan_image_available_semaphore = VK_NULL_HANDLE;
		}
		if (vulkan_rendering_finished_semaphore != VK_NULL_HANDLE) {
			vulkanapi::vkDestroySemaphore(vulkan_device, vulkan_rendering_finished_semaphore, nullptr);
			vulkan_rendering_finished_semaphore = VK_NULL_HANDLE;
		}
		if (vulkan_swapchain != VK_NULL_HANDLE) {
			vulkanapi::vkDestroySwapchainKHR(vulkan_device, vulkan_swapchain, nullptr);
			vulkan_swapchain = VK_NULL_HANDLE;
		}

		// do this last
		vulkanapi::vkDestroyDevice(vulkan_device, nullptr);
		vulkan_device = VK_NULL_HANDLE;
	}


	if (vulkan_instance != VK_NULL_HANDLE) {
		if (vulkan_presentation_surface != VK_NULL_HANDLE) {
			vulkanapi::vkDestroySurfaceKHR(vulkan_instance, vulkan_presentation_surface, nullptr);
			vulkan_presentation_surface = VK_NULL_HANDLE;
		}

		vulkanapi::vkDestroyInstance(vulkan_instance, nullptr);
		vulkan_instance = VK_NULL_HANDLE;
	}

	// delete vectors
	//std::vector<VkBuffer> vulkan_buffers(0);
	//std::vector<VkCommandBuffer> vulkan_PresentQueueCmdBuffers(0);
	vulkan_buffers.clear();
	vulkan_PresentQueueCmdBuffers.clear();


	if (VulkanLibrary) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		FreeLibrary(VulkanLibrary);
#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR)
		dlclose(VulkanLibrary);
#endif
	}

}


// it is possible for vulkan to request the swap chain be recreated, like if window size changes etc
// swap chain is used to show the output in the main program window
bool vulkan_create_swap_chain()
{
	cout << "\nVULKAN: INFO: setting up swap chain\n";


	// clear previous swap chain data
	vulkan_clear();


	VkCommandPoolCreateInfo commandPoolCreateInfo = {
	  VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	  0,
	  0,
	  vulkan_queueFamilyIndex
	};

	//VkCommandPool vulkan_commandPool;
	if (vulkanapi::vkCreateCommandPool(vulkan_device, &commandPoolCreateInfo, 0, &vulkan_commandPool) != VK_SUCCESS)
	{
		cout << "\nVULKAN ERROR: Problem creating command pool.\n";
		return false;
	}


	// get surface capabilities
	VkSurfaceCapabilitiesKHR surface_capabilities;
	if (vulkanapi::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_physicalDevice, vulkan_presentation_surface, &surface_capabilities) != VK_SUCCESS) {
		std::cout << "ERROR: VULKAN: Could not check presentation surface capabilities!" << std::endl;
		return false;
	}


	// get supported surface formats
	uint32_t formats_count;
	if ((vulkanapi::vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_physicalDevice, vulkan_presentation_surface, &formats_count, nullptr) != VK_SUCCESS) ||
		(formats_count == 0)) {
		std::cout << "ERROR: VULKAN: occurred during presentation surface formats enumeration!" << std::endl;
		return false;
	}

	std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
	if (vulkanapi::vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_physicalDevice, vulkan_presentation_surface, &formats_count, &surface_formats[0]) != VK_SUCCESS) {
		std::cout << "ERROR: VULKAN: occurred during presentation surface formats enumeration!" << std::endl;
		return false;
	}


	// get supported present modes
	uint32_t present_modes_count;
	if ((vulkanapi::vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_physicalDevice, vulkan_presentation_surface, &present_modes_count, nullptr) != VK_SUCCESS) ||
		(present_modes_count == 0)) {
		std::cout << "Error occurred during presentation surface present modes enumeration!" << std::endl;
		return false;
	}

	std::vector<VkPresentModeKHR> present_modes(present_modes_count);
	if (vulkanapi::vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_physicalDevice, vulkan_presentation_surface, &present_modes_count, &present_modes[0]) != VK_SUCCESS) {
		std::cout << "Error occurred during presentation surface present modes enumeration!" << std::endl;
		return false;
	}



	// Set of images defined in a swap chain may not always be available for application to render to:
	// One may be displayed and one may wait in a queue to be presented
	// If application wants to use more images at the same time it must ask for more images
	uint32_t image_count = surface_capabilities.minImageCount + 1;
	if ((surface_capabilities.maxImageCount > 0) &&
		(image_count > surface_capabilities.maxImageCount)) {
		image_count = surface_capabilities.maxImageCount;
	}

	//std::cout << "\nVULKAN: number of swapchain images: " << dec << image_count << " out of a max of: " << surface_capabilities.maxImageCount;

	// select format for swap chain images
	// If the list contains only one entry with undefined format
	// it means that there are no preferred surface formats and any can be chosen
	VkSurfaceFormatKHR surface_format;
	if ((surface_formats.size() == 1) &&
		(surface_formats[0].format == VK_FORMAT_UNDEFINED)) {
		surface_format = { VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
	}


	// Check if list contains most widely used R8 G8 B8 A8 format
	// with nonlinear color space
	//for (/* VkSurfaceFormatKHR& */ surface_format : surface_formats) {
	for (int i = 0; i < surface_formats.size(); i++)
	{
		if (surface_formats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
		{
			//return surface_format;
			surface_format = surface_formats[i];
			break;
		}
	}


	if (surface_format.format != VK_FORMAT_R8G8B8A8_UNORM)
	{
		std::cout << "\nERROR: VULKAN: unable to choose desired swap chain surface format.";

		surface_format = surface_formats[0];
	}



	// Return the first format from the list
	//return surface_formats[0];


	// select size of swap chain images
	// Special value of surface extent is width == height == -1
	// If this is so we define the size by ourselves but it must fit within defined confines
	VkExtent2D swap_chain_extent;
	if (surface_capabilities.currentExtent.width == -1) {

		// TODO: this should probably be the size of the program window - not constant
		//swap_chain_extent = { 640, 480 };
		swap_chain_extent = { vulkan_screen_size_x, vulkan_screen_size_y };

		if (swap_chain_extent.width < surface_capabilities.minImageExtent.width) {
			swap_chain_extent.width = surface_capabilities.minImageExtent.width;
		}
		if (swap_chain_extent.height < surface_capabilities.minImageExtent.height) {
			swap_chain_extent.height = surface_capabilities.minImageExtent.height;
		}
		if (swap_chain_extent.width > surface_capabilities.maxImageExtent.width) {
			swap_chain_extent.width = surface_capabilities.maxImageExtent.width;
		}
		if (swap_chain_extent.height > surface_capabilities.maxImageExtent.height) {
			swap_chain_extent.height = surface_capabilities.maxImageExtent.height;
		}
		//return swap_chain_extent;
	}
	else
	{
		swap_chain_extent = surface_capabilities.currentExtent;
	}

	// Most of the cases we define size of the swap_chain images equal to current window's size
	//return surface_capabilities.currentExtent;



	// Color attachment flag must always be supported
	// We can define other usage flags but we always need to check if they are supported
	VkImageUsageFlags surface_flags = static_cast<VkImageUsageFlags>(-1);
	if (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		surface_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	else
	{
		std::cout << "ERROR: VULKAN: VK_IMAGE_USAGE_TRANSFER_DST image usage is not supported by the swap chain!" << std::endl
			<< "Supported swap chain's image usages include:" << std::endl
			<< (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT ? "    VK_IMAGE_USAGE_TRANSFER_SRC\n" : "")
			<< (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT ? "    VK_IMAGE_USAGE_TRANSFER_DST\n" : "")
			<< (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT ? "    VK_IMAGE_USAGE_SAMPLED\n" : "")
			<< (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT ? "    VK_IMAGE_USAGE_STORAGE\n" : "")
			<< (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ? "    VK_IMAGE_USAGE_COLOR_ATTACHMENT\n" : "")
			<< (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? "    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT\n" : "")
			<< (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT ? "    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT\n" : "")
			<< (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT ? "    VK_IMAGE_USAGE_INPUT_ATTACHMENT" : "")
			<< std::endl;
	}

	//return static_cast<VkImageUsageFlags>(-1);



	// Sometimes images must be transformed before they are presented (i.e. due to device's orienation
	// being other than default orientation)
	// If the specified transform is other than current transform, presentation engine will transform image
	// during presentation operation; this operation may hit performance on some platforms
	// Here we don't want any transformations to occur so if the identity transform is supported use it
	// otherwise just use the same transform as current transform
	VkSurfaceTransformFlagBitsKHR surface_transform;
	if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		//if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) {
			//if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR) {
			//return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		surface_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		//surface_transform = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR;
		//surface_transform = VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR;
	}
	else {
		//return surface_capabilities.currentTransform;
		surface_transform = surface_capabilities.currentTransform;
	}



	// TODO - CONTINUE FROM HERE
	// FIFO present mode is always available
	// MAILBOX is the lowest latency V-Sync enabled mode (something like triple-buffering) so use it if available
	VkPresentModeKHR present_mode = static_cast<VkPresentModeKHR>(-1);
	//for (VkPresentModeKHR& present_mode : present_modes) {
	for (int i = 0; i < present_modes.size(); i++)
	{
		if (present_modes[i] == VK_PRESENT_MODE_FIFO_KHR)
		{
			present_mode = present_modes[i];
			//return present_mode;
		}


		//if (present_modes[i] == VK_PRESENT_MODE_FIFO_KHR)
		if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			present_mode = present_modes[i];

			// prefer mailbox present mode over fifo present mode
			break;
			//return present_mode;
		}
	}

	std::cout << "\npresent_mode= " << present_mode;

	if (present_mode == static_cast<VkPresentModeKHR>(-1))
	{
		std::cout << "ERROR: VULKAN: FIFO present mode is not supported by the swap chain!" << std::endl;
		//return static_cast<VkPresentModeKHR>(-1);
		return false;
	}



	// create swap chain
	uint32_t                      desired_number_of_images = image_count;	// GetSwapChainNumImages(surface_capabilities);
	VkSurfaceFormatKHR            desired_format = surface_format;	// GetSwapChainFormat(surface_formats);
	VkExtent2D                    desired_extent = swap_chain_extent;	// GetSwapChainExtent(surface_capabilities);
	VkImageUsageFlags             desired_usage = surface_flags;	// GetSwapChainUsageFlags(surface_capabilities);
	VkSurfaceTransformFlagBitsKHR	desired_transform = surface_transform;	// GetSwapChainTransform(surface_capabilities);
	VkPresentModeKHR              desired_present_mode = present_mode;	// GetSwapChainPresentMode(present_modes);
	VkSwapchainKHR                old_swap_chain = vulkan_swapchain;	// Vulkan.SwapChain;

	//if (static_cast<int>(desired_usage) == -1) {
	//	return false;
	//}
	//if (static_cast<int>(desired_present_mode) == -1) {
	//	return false;
	//}

	VkSwapchainCreateInfoKHR swap_chain_create_info = {
	  VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // VkStructureType                sType
	  nullptr,                                      // const void                    *pNext
	  0,                                            // VkSwapchainCreateFlagsKHR      flags
	  vulkan_presentation_surface,                   // VkSurfaceKHR                   surface
	  desired_number_of_images,                     // uint32_t                       minImageCount
	  desired_format.format,                        // VkFormat                       imageFormat
	  desired_format.colorSpace,                    // VkColorSpaceKHR                imageColorSpace
	  desired_extent,                               // VkExtent2D                     imageExtent
	  1,                                            // uint32_t                       imageArrayLayers
	  desired_usage,                                // VkImageUsageFlags              imageUsage
	  VK_SHARING_MODE_EXCLUSIVE,                    // VkSharingMode                  imageSharingMode
	  0,                                            // uint32_t                       queueFamilyIndexCount
	  nullptr,                                      // const uint32_t                *pQueueFamilyIndices
	  desired_transform,                            // VkSurfaceTransformFlagBitsKHR  preTransform
	  VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,            // VkCompositeAlphaFlagBitsKHR    compositeAlpha
	  desired_present_mode,                         // VkPresentModeKHR               presentMode
	  VK_TRUE,                                      // VkBool32                       clipped
	  old_swap_chain                                // VkSwapchainKHR                 oldSwapchain
	};

	//VkSwapchainKHR vulkan_swapchain;
	if (vulkanapi::vkCreateSwapchainKHR(vulkan_device, &swap_chain_create_info, nullptr, &vulkan_swapchain) != VK_SUCCESS) {
		std::cout << "ERROR: VULKAN: Could not create swap chain!" << std::endl;
		return false;
	}
	if (old_swap_chain != VK_NULL_HANDLE) {
		vulkanapi::vkDestroySwapchainKHR(vulkan_device, old_swap_chain, nullptr);
	}

	//return true;


	// get the buffer to use for staging, which is going to be the last one
	VkBuffer vulkan_staging_buffer = vulkan_buffers[vulkan_buffers.size() - 1];


	// get the images created in the swap chain
	std::vector<VkImage> swap_chain_images(image_count);
	if (vulkanapi::vkGetSwapchainImagesKHR(vulkan_device, vulkan_swapchain, &image_count, &swap_chain_images[0]) != VK_SUCCESS) {
		std::cout << "ERROR: VULKAN: Could not get swap chain images!" << std::endl;
		return false;
	}


	VkImageSubresourceRange image_subresource_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	VkClearColorValue clear_color = {
		{ 1.0f, 0.8f, 0.4f, 0.0f }
	};


	VkImageBlit image_blit_region = {
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		{ { 0, 0, 0 }, { 640, 480, 1 } },
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		//{ { 0, 0, 0 }, { 640, 480, 1 } }
		{ { 0, 0, 0 }, { vulkan_screen_size_x, vulkan_screen_size_y, 1 } }
	};


	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		640,	// width
		480,	// height
		1
	};


	// image barrier for the image that compute shader writes to
	VkImageMemoryBarrier barrier_wait_until_image_written = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
		nullptr,                                    // const void                            *pNext
		VK_ACCESS_SHADER_WRITE_BIT,                  // VkAccessFlags                          srcAccessMask
		VK_ACCESS_TRANSFER_READ_BIT,               // VkAccessFlags                          dstAccessMask
		VK_IMAGE_LAYOUT_GENERAL,                  // VkImageLayout                          oldLayout
		VK_IMAGE_LAYOUT_GENERAL,				// VkImageLayout                          newLayout
		vulkan_queueFamilyIndex,             // uint32_t                               srcQueueFamilyIndex
		vulkan_queueFamilyIndex,             // uint32_t                               dstQueueFamilyIndex
		vulkan_image,                       // VkImage                                image
		image_subresource_range                     // VkImageSubresourceRange                subresourceRange
	};

	VkBufferMemoryBarrier barrier_wait_until_buffer_written = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		vulkan_queueFamilyIndex,             // uint32_t                               srcQueueFamilyIndex
		vulkan_queueFamilyIndex,             // uint32_t                               dstQueueFamilyIndex
		vulkan_staging_buffer,
		0,
		640 * 480 * 4
	};


	VkBufferMemoryBarrier barrier_wait_until_buffer_read = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		vulkan_queueFamilyIndex,             // uint32_t                               srcQueueFamilyIndex
		vulkan_queueFamilyIndex,             // uint32_t                               dstQueueFamilyIndex
		vulkan_staging_buffer,
		0,
		640 * 480 * 4
	};

	VkMemoryBarrier barrier_wait_until_memory_written = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_TRANSFER_READ_BIT
	};

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
	  VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	  0,
	  vulkan_commandPool,
	  VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	  1
	};

	VkCommandBufferBeginInfo commandBufferBeginInfo = {
	  VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	  0,
	  VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
	  0
	};

	std::cout << "\nVULKAN: INFO: creating swap chain command buffers\n";

	// create command buffers for vulkan surfaces
	vulkan_PresentQueueCmdBuffers.resize(image_count);
	for (int i = 0; i < image_count; i++)
	{
		VkImageMemoryBarrier barrier_from_present_to_clear = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
			nullptr,                                    // const void                            *pNext
			VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                          srcAccessMask
			VK_ACCESS_TRANSFER_WRITE_BIT,               // VkAccessFlags                          dstAccessMask
			VK_IMAGE_LAYOUT_UNDEFINED,                  // VkImageLayout                          oldLayout
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       // VkImageLayout                          newLayout
			vulkan_queueFamilyIndex,             // uint32_t                               srcQueueFamilyIndex
			vulkan_queueFamilyIndex,             // uint32_t                               dstQueueFamilyIndex
			swap_chain_images[i],                       // VkImage                                image
			image_subresource_range                     // VkImageSubresourceRange                subresourceRange
		};

		VkImageMemoryBarrier barrier_from_clear_to_present = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
			nullptr,                                    // const void                            *pNext
			VK_ACCESS_TRANSFER_WRITE_BIT,               // VkAccessFlags                          srcAccessMask
			VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                          dstAccessMask
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       // VkImageLayout                          oldLayout
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            // VkImageLayout                          newLayout
			vulkan_queueFamilyIndex,             // uint32_t                               srcQueueFamilyIndex
			vulkan_queueFamilyIndex,             // uint32_t                               dstQueueFamilyIndex
			swap_chain_images[i],                       // VkImage                                image
			image_subresource_range                     // VkImageSubresourceRange                subresourceRange
		};

		if (vulkanapi::vkAllocateCommandBuffers(vulkan_device, &commandBufferAllocateInfo, &vulkan_PresentQueueCmdBuffers[i]) != VK_SUCCESS)
		{
			std::cout << "\nVULKAN ERROR: Problem allocating command buffers for swap chain commands.\n";
			return false;
		}

		vulkanapi::vkBeginCommandBuffer(vulkan_PresentQueueCmdBuffers[i], &commandBufferBeginInfo);

		//vulkanapi::vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &barrier_wait_until_buffer_read, 0, nullptr);

		VkBufferCopy bufcopy =
		{
			0, 0, 1024 * 512 * 2
		};
		//vkCmdCopyBuffer(vulkan_PresentQueueCmdBuffers[i], vulkan_out_buffer, vulkan_inout_buffer, 1, &bufcopy);

		// I'd imagine the compute shader run should go here
		vulkanapi::vkCmdBindPipeline(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, vulkan_pipeline);

		vulkanapi::vkCmdBindDescriptorSets(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, vulkan_pipelineLayout, 0, 1, &vulkan_descriptorSet, 0, 0);

		// set number of shaders to run simultaneously here
		vulkanapi::vkCmdDispatch(vulkan_PresentQueueCmdBuffers[i], vulkan_num_gpu_x_workgroups, 1, 1);

		//vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &barrier_wait_until_buffer_written, 0, nullptr);

		// wait for all previous commands to finish
		//vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &barrier_wait_until_buffer_written, 0, nullptr);




		//vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_wait_until_image_written);
		//vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &barrier_wait_until_buffer_written, 0, nullptr);


		// draw to frame buffer
		//vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_present_to_clear);
		//vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_present_to_clear);
		//vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_present_to_clear);
		vulkanapi::vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, &barrier_wait_until_buffer_written, 0, &barrier_from_present_to_clear);

		//vkCmdClearColorImage(vulkan_PresentQueueCmdBuffers[i], swap_chain_images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &image_subresource_range);

		//vkCmdCopyBuffer(vulkan_PresentQueueCmdBuffers[i], vulkan_inout_buffer, vulkan_out_buffer, 1, &bufcopy);


		// draw the screen from buffer
		//vulkanapi::vkCmdCopyBufferToImage(vulkan_PresentQueueCmdBuffers[i], vulkan_staging_buffer, swap_chain_images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		vulkanapi::vkCmdCopyBufferToImage(vulkan_PresentQueueCmdBuffers[i], vulkan_staging_buffer, vulkan_image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

		// blit image to the screen
		vulkanapi::vkCmdBlitImage(vulkan_PresentQueueCmdBuffers[i], vulkan_image, VK_IMAGE_LAYOUT_GENERAL, swap_chain_images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_blit_region, VK_FILTER_NEAREST);

		VkImageMemoryBarrier barrier_wait_until_screen_written = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
			nullptr,                                    // const void                            *pNext
			VK_ACCESS_TRANSFER_WRITE_BIT,                  // VkAccessFlags                          srcAccessMask
			VK_ACCESS_TRANSFER_READ_BIT,               // VkAccessFlags                          dstAccessMask
			VK_IMAGE_LAYOUT_UNDEFINED,                  // VkImageLayout                          oldLayout
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,				// VkImageLayout                          newLayout
			vulkan_queueFamilyIndex,             // uint32_t                               srcQueueFamilyIndex
			vulkan_queueFamilyIndex,             // uint32_t                               dstQueueFamilyIndex
			swap_chain_images[i],                       // VkImage                                image
			image_subresource_range                     // VkImageSubresourceRange                subresourceRange
		};

		// wait for all previous commands to finish
		//vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_wait_until_screen_written);

		
		//vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_clear_to_present);
		//vulkanapi::vkCmdPipelineBarrier(vulkan_PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_clear_to_present);

		if (vulkanapi::vkEndCommandBuffer(vulkan_PresentQueueCmdBuffers[i]) != VK_SUCCESS) {
			std::cout << "ERROR: VULKAN: Could not record command buffers!" << std::endl;
			return false;
		}

	}	// end for (int i = 0; i < image_count; i++)



	std::cout << "\nVULKAN: INFO: creating second set of command buffers.\n";


	/*
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
	  VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	  0,
	  vulkan_commandPool,
	  VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	  1
	};
	VkCommandBufferBeginInfo commandBufferBeginInfo = {
	  VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	  0,
	  VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
	  0
	};
	*/

	//VkCommandBuffer commandBuffer;
	if (vulkanapi::vkAllocateCommandBuffers(vulkan_device, &commandBufferAllocateInfo, &vulkan_commandBuffer) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem allocating command buffers.\n";
		return nullptr;

	}




	if (vulkanapi::vkBeginCommandBuffer(vulkan_commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem with begin command buffer.\n";
		return nullptr;

	}

	//vkCmdPipelineBarrier(vulkan_commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

	vulkanapi::vkCmdBindPipeline(vulkan_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vulkan_pipeline);

	vulkanapi::vkCmdBindDescriptorSets(vulkan_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		vulkan_pipelineLayout, 0, 1, &vulkan_descriptorSet, 0, 0);

	// set number of shaders to run simultaneously here
	vulkanapi::vkCmdDispatch(vulkan_commandBuffer, vulkan_num_gpu_x_workgroups, 1, 1);


	//vkCmdCopyBuffer(vulkan_commandBuffer, vulkan_inout_buffer, vulkan_out_buffer, 1, &bufcopy);


	//vkCmdPipelineBarrier(Vulkan.PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_clear_to_present);
	//vkCmdPipelineBarrier(vulkan_commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_wait_until_image_written);

	if (vulkanapi::vkEndCommandBuffer(vulkan_commandBuffer) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem with end command buffer.\n";
		return nullptr;

	}




	// delete vectors
	//std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
	//std::vector<VkPresentModeKHR> present_modes(present_modes_count);
	//std::vector<VkImage> swap_chain_images(image_count);
	surface_formats.clear();
	present_modes.clear();
	swap_chain_images.clear();

	// ok
	return true;

}



// returns a pointer into the memory for device
// shader_file_name - name of the file that contains the computer shader
// inout_buffer_size_bytes - size of ps1/ps2 gpu ram (1MB or 4MB, etc)
// out_buffer_size_bytes - size of ps1/ps2 screen output
// in_comm_buffer_size_bytes - size of input draw commands buffer
// in_data_buffer_size_bytes - size of input data buffer (graphics data)
// work_buffer_size_bytes - size of work buffer for rasterizing primitives (number of input command buffer slots times data needed per slot)
//void* vulkan_setup(HWND wHandle, HINSTANCE wInstance, char* shader_file_data, int shader_file_size, int inout_buffer_size_bytes, int out_buffer_size_bytes, int in_comm_buffer_size_bytes, int in_data_buffer_size_bytes, int work_buffer_size_bytes)
void* vulkan_setup(HWND wHandle, HINSTANCE wInstance, char* shader_file_data, int shader_file_size, long* external_vars_sizes, int external_vars_count, long* internal_vars_sizes, int internal_vars_count)
{
	// dynamically load vulkan //

	if (!vulkan_load_library())
	{
		std::cout << "\nERROR: VULKAN: INIT: Unable to load library!\n";
		return nullptr;
	}

	if (!vulkanapi::vulkan_load_exported_entry_points())
	{
		std::cout << "\nERROR: VULKAN: INIT: Unable to load exported entry points!\n";
		return nullptr;

	}

	if (!vulkanapi::vulkan_load_global_level_entry_points())
	{
		std::cout << "\nERROR: VULKAN: INIT: Unable to load global level entry points!\n";
		return nullptr;

	}




	// swap chain not created yet, so no previous swap chain
	vulkan_swapchain = VK_NULL_HANDLE;


	std::vector<const char*> required_instance_extensions = {
	  VK_KHR_SURFACE_EXTENSION_NAME,
	#if defined(VK_USE_PLATFORM_WIN32_KHR)
	  VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	#elif defined(VK_USE_PLATFORM_XCB_KHR)
	  VK_KHR_XCB_SURFACE_EXTENSION_NAME
	#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	  VK_KHR_XLIB_SURFACE_EXTENSION_NAME
	#endif
	};


	uint32_t extensionCount = 0;
	vulkanapi::vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::cout << "\n" << extensionCount << " extensions supported\n";

	std::vector<VkExtensionProperties> extensions(extensionCount);

	vulkanapi::vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "available extensions:\n";

	for (const auto& extension : extensions) {
		std::cout << '\t' << extension.extensionName << '\n';
	}


	const VkApplicationInfo applicationInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		0,
		"VKComputeSample",
		0,
		"",
		0,
		VK_MAKE_VERSION(1, 1, 0)
	};




	const VkInstanceCreateInfo instanceCreateInfo = {
	  VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	  0,
	  0,
	  &applicationInfo,
	  0,
	  0,

	  // TODO: validate extension availability
	  // enable all the extensions, in case we need to use some (like swap chain for double buffering, etc)
	  static_cast<uint32_t>(required_instance_extensions.size()),
	  required_instance_extensions.data()
	};

	//VkInstance vulkan_instance;
	if (vulkanapi::vkCreateInstance(&instanceCreateInfo, 0, &vulkan_instance) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem creating VULKAN instance.\n";
		return nullptr;
	}


	if (!vulkanapi::vulkan_load_instance_level_entry_points())
	{
		std::cout << "\nERROR: VULKAN: INIT: Unable to load instance level entry points!\n";
		return nullptr;

	}




	// create presentation surface for WINDOWS OS
	VkWin32SurfaceCreateInfoKHR surface_create_info = {
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,  // VkStructureType                  sType
		nullptr,                                          // const void                      *pNext
		0,                                                // VkWin32SurfaceCreateFlagsKHR     flags
		wInstance,                                  // HINSTANCE                        hinstance
		wHandle                                     // HWND                             hwnd
	};

	//VkSurfaceKHR vulkan_presentation_surface;
	if (vulkanapi::vkCreateWin32SurfaceKHR(vulkan_instance, &surface_create_info, nullptr, &vulkan_presentation_surface) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem creating WIN32 surface.\n";
		return nullptr;
	}



	uint32_t physicalDeviceCount = 0;
	if (vulkanapi::vkEnumeratePhysicalDevices(vulkan_instance, &physicalDeviceCount, 0) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem enumerating VULKAN physical devices.\n";
		return nullptr;
	}

	//VkPhysicalDevice* const physicalDevices = (VkPhysicalDevice*)malloc(
	//	sizeof(VkPhysicalDevice) * physicalDeviceCount);
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

	if (vulkanapi::vkEnumeratePhysicalDevices(vulkan_instance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem enumerating VULKAN physical devices.\n";
		return nullptr;
	}

	std::cout << "\n" << physicalDeviceCount << " physical devices supported\n";


	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;

	VkPhysicalDeviceSubgroupProperties subgroupProperties;
	VkPhysicalDeviceProperties2 physicalDeviceProperties;

	std::cout << "\navailable devices:\n";

	//VkPhysicalDevice vulkan_physicalDevice;

	for (const auto& device : physicalDevices) {

		vulkanapi::vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vulkanapi::vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		subgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
		subgroupProperties.pNext = NULL;

		physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		physicalDeviceProperties.pNext = &subgroupProperties;

		vulkanapi::vkGetPhysicalDeviceProperties2(device, &physicalDeviceProperties);

		std::cout << '\t' << deviceProperties.deviceName << '\n';
		std::cout << '\t' << "devicetype:" << deviceProperties.deviceType << '\n';
		std::cout << '\t' << "apiVersion:" << deviceProperties.apiVersion << '\n';
		std::cout << '\t' << "maxComputeSharedMemorySize:" << deviceProperties.limits.maxComputeSharedMemorySize << '\n';
		std::cout << '\t' << "maxComputeWorkGroupCount:" << deviceProperties.limits.maxComputeWorkGroupCount << '\n';
		std::cout << '\t' << "maxComputeWorkGroupInvocations:" << deviceProperties.limits.maxComputeWorkGroupInvocations << '\n';
		std::cout << '\t' << "maxComputeWorkGroupSize:" << deviceProperties.limits.maxComputeWorkGroupSize << '\n';
		std::cout << '\t' << "maxDescriptorSetStorageBuffers:" << deviceProperties.limits.maxDescriptorSetStorageBuffers << '\n';
		
		std::cout << '\t' << "subgroupSize: " << subgroupProperties.subgroupSize;

		if ((deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) || (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU))
		{
			std::cout << "\n***Found suitable device***\n";
			vulkan_physicalDevice = device;

			break;
		}
	}


	// record subgroup size
	vulkan_SubgroupSize = subgroupProperties.subgroupSize;

	uint32_t queueFamilyPropertiesCount = 0;
	vulkanapi::vkGetPhysicalDeviceQueueFamilyProperties(vulkan_physicalDevice, &queueFamilyPropertiesCount, 0);

	//VkQueueFamilyProperties* const queueFamilyProperties = (VkQueueFamilyProperties*)_alloca(
	//	sizeof(VkQueueFamilyProperties) * queueFamilyPropertiesCount);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);

	vulkanapi::vkGetPhysicalDeviceQueueFamilyProperties(vulkan_physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());

	//uint32_t queueFamilyIndex;
	VkBool32 queue_present_support;
	//for (const auto& properties : queueFamilyProperties) {
	for (int i = 0; i < queueFamilyPropertiesCount; i++)
	{
		// mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
		//const VkQueueFlags maskedFlags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) & queueFamilyProperties[i].queueFlags);
		const VkQueueFlags maskedFlags = queueFamilyProperties[i].queueFlags;

		//if (!(VK_QUEUE_GRAPHICS_BIT & maskedFlags) && (VK_QUEUE_COMPUTE_BIT & maskedFlags)) {
		if (
			(VK_QUEUE_COMPUTE_BIT & maskedFlags)

			//&& (VK_QUEUE_TRANSFER_BIT & maskedFlags)

			// can toggle/modify this for testing
			&& (VK_QUEUE_GRAPHICS_BIT & maskedFlags)
			)
		{

			// this is the queueFamilyIndex for the compute queue, the one for the graphics queue (if there is one) could be different
			vulkan_queueFamilyIndex = i;

			// need to know if presentation is supported
			vulkanapi::vkGetPhysicalDeviceSurfaceSupportKHR(vulkan_physicalDevice, i, vulkan_presentation_surface, &queue_present_support);

			std::cout << "\n***Found suitable queue family index for COMPUTE queue***\n";

			// choose this one right away if it also has presentation support
			if (queue_present_support)
			{
				std::cout << "\n***Found suitable queue family index for COMPUTE queue with PRESENTATION support***\n";
				break;
			}
		}
	}

	VkDeviceQueueCreateInfo deviceQueueCreateInfo;

	float queuePriority = 1.0f;
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueFamilyIndex = vulkan_queueFamilyIndex;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
	deviceQueueCreateInfo.flags = 0;
	deviceQueueCreateInfo.pNext = 0;


	std::vector<const char*> required_device_extensions = {
	  VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};


	VkDeviceCreateInfo deviceCreateInfo;

	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;

	// enable all the device features
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	//deviceCreateInfo.pEnabledFeatures = 0;


	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = required_device_extensions.data();
	//deviceCreateInfo.enabledExtensionCount = 0;
	//deviceCreateInfo.ppEnabledExtensionNames = nullptr;

	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.pNext = 0;


	//VkDevice vulkan_device;
	if (vulkanapi::vkCreateDevice(vulkan_physicalDevice, &deviceCreateInfo, nullptr, &vulkan_device) != VK_SUCCESS) {
		//throw std::runtime_error("failed to create logical device!");
		std::cout << "\nVULKAN ERROR: Problem creating VULKAN device.\n";
		return nullptr;
	}
	else
	{
		std::cout << "\nVULKAN SUCCESS: Successfully created VULKAN device.\n";

	}



	if (!vulkanapi::vulkan_load_device_level_entry_points())
	{
		std::cout << "\nERROR: VULKAN: INIT: Unable to load device level entry points!\n";
		return nullptr;

	}


	VkQueue computeQueue;
	vulkanapi::vkGetDeviceQueue(vulkan_device, vulkan_queueFamilyIndex, 0, &computeQueue);

	// *** device created *** //


	// create semaphores
	VkSemaphoreCreateInfo semaphore_create_info = {
	  VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,      // VkStructureType          sType
	  nullptr,                                      // const void*              pNext
	  0                                             // VkSemaphoreCreateFlags   flags
	};

	//VkSemaphore vulkan_image_available_semaphore;
	//VkSemaphore vulkan_rendering_finished_semaphore;
	if ((vulkanapi::vkCreateSemaphore(vulkan_device, &semaphore_create_info, nullptr, &vulkan_image_available_semaphore) != VK_SUCCESS) ||
		(vulkanapi::vkCreateSemaphore(vulkan_device, &semaphore_create_info, nullptr, &vulkan_rendering_finished_semaphore) != VK_SUCCESS)) {

		std::cout << "ERROR: VULKAN: Could not create semaphores!" << std::endl;
		return nullptr;
	}






	VkPhysicalDeviceMemoryProperties properties;

	vulkanapi::vkGetPhysicalDeviceMemoryProperties(vulkan_physicalDevice, &properties);

	
	VkImageCreateInfo vulkan_image_info =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		0,
		VK_IMAGE_TYPE_2D,

		// I want a 24-bit rgb format, but TODO - need to check to make sure format is supported
		//VK_FORMAT_R32G32B32A32_SFLOAT,	//VK_FORMAT_R8G8B8A8_UINT,
		VK_FORMAT_R8G8B8A8_UNORM,

		// size of the image
		{ 640, 640, 1 },

		1, 1,
		VK_SAMPLE_COUNT_1_BIT,
		//VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_TILING_OPTIMAL,

		// just storing image for use in compute shader for now
		VK_IMAGE_USAGE_STORAGE_BIT,

		// only using compute queue for now
		VK_SHARING_MODE_EXCLUSIVE,

		// this is just going to be on the compute queue for now, TODO might also need to add graphics queue later for displaying onscreen?
		1, &vulkan_queueFamilyIndex,

		VK_IMAGE_LAYOUT_GENERAL
	};


	//VkImage vulkan_image;
	if (vulkanapi::vkCreateImage(vulkan_device, &vulkan_image_info, nullptr, &vulkan_image) != VK_SUCCESS)
	{
		cout << "\nERROR: VULKAN: problem creating image.";
	}

	VkMemoryRequirements image_memReqs;
	vulkanapi::vkGetImageMemoryRequirements(vulkan_device, vulkan_image, &image_memReqs);
	


	//const int32_t bufferLength = 16;

	//const uint32_t bufferSize = sizeof(int32_t) * bufferLength;
	//const uint32_t inout_bufferSize = inout_buffer_size_bytes;
	//const uint32_t out_bufferSize = out_buffer_size_bytes;
	//const uint32_t incomm_bufferSize = in_comm_buffer_size_bytes;
	//const uint32_t indata_bufferSize = in_data_buffer_size_bytes;
	//const uint32_t work_bufferSize = work_buffer_size_bytes;
	//const uint32_t staging_bufferSize = 640 * 480 * 4;


	// we are going to need two buffers from this one memory
	//const VkDeviceSize memorySize = bufferSize * 2;
	//const VkDeviceSize memorySize = inout_buffer_size_bytes + out_bufferSize + incomm_bufferSize + indata_bufferSize;	// +work_bufferSize + image_memReqs.size;
	//const VkDeviceSize memorySize = SCRATCH_SPACE_SIZE_BYTES + inout_buffer_size_bytes + out_bufferSize + incomm_bufferSize + indata_bufferSize;	// +work_bufferSize + image_memReqs.size;

	//const VkDeviceSize memorySize_local = work_bufferSize + staging_bufferSize;
	//const VkDeviceSize memorySize_local = work_bufferSize + staging_bufferSize + inout_bufferSize;

	int total_vars_count = 0;
	std::vector<VkDeviceSize> vulkan_bufferSizes(external_vars_count + internal_vars_count);

	// add up external memory total size
	VkDeviceSize memorySize = 0;
	for (int i = 0; i < external_vars_count; i++)
	{
		vulkan_bufferSizes[total_vars_count++] = external_vars_sizes[i];
		memorySize += external_vars_sizes[i];
	}

	// add up internal memory total size
	VkDeviceSize memorySize_local = 0;
	for (int i = 0; i < internal_vars_count; i++)
	{
		vulkan_bufferSizes[total_vars_count++] = internal_vars_sizes[i];
		memorySize_local += internal_vars_sizes[i];
	}

	// set memoryTypeIndex to an invalid entry in the properties.memoryTypes array
	uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;
	for (uint32_t k = 0; k < properties.memoryTypeCount; k++) {
		if (
			//(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT & properties.memoryTypes[k].propertyFlags) &&
			(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & properties.memoryTypes[k].propertyFlags) &&
			(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & properties.memoryTypes[k].propertyFlags) &&
			(VK_MEMORY_PROPERTY_HOST_CACHED_BIT & properties.memoryTypes[k].propertyFlags) &&
			(memorySize < properties.memoryHeaps[properties.memoryTypes[k].heapIndex].size)) {
			memoryTypeIndex = k;
			break;
		}
	}


	if (memoryTypeIndex == VK_MAX_MEMORY_TYPES)
	{
		std::cout << "\nERROR: VULKAN: Problem selecting external memory type.\n";
		return nullptr;
	}


	uint32_t memoryTypeIndex_local = VK_MAX_MEMORY_TYPES;
	for (uint32_t k = 0; k < properties.memoryTypeCount; k++) {
		if (
			(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT & properties.memoryTypes[k].propertyFlags) &&
			//(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & properties.memoryTypes[k].propertyFlags) &&
			//(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & properties.memoryTypes[k].propertyFlags) &&
			//(VK_MEMORY_PROPERTY_HOST_CACHED_BIT & properties.memoryTypes[k].propertyFlags) &&
			(memorySize_local < properties.memoryHeaps[properties.memoryTypes[k].heapIndex].size)) {
			memoryTypeIndex_local = k;
			break;
		}
	}

	if (memoryTypeIndex_local == VK_MAX_MEMORY_TYPES)
	{
		std::cout << "\nERROR: VULKAN: Problem selecting local memory type.\n";
		return nullptr;
	}



	cout << "\nmemoryTypeIndex=" << dec << memoryTypeIndex << " memoryTypeCount=" << properties.memoryTypeCount << " VK_MAX_MEMORY_TYPES=" << VK_MAX_MEMORY_TYPES;



	const VkMemoryAllocateInfo memoryAllocateInfo = {
		  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		  0,
		  memorySize,
		  memoryTypeIndex
	};

	//VkDeviceMemory vulkan_memory;
	if (vulkanapi::vkAllocateMemory(vulkan_device, &memoryAllocateInfo, 0, &vulkan_memory) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem allocating memory for device.\n";
		return nullptr;

	}


	const VkMemoryAllocateInfo memoryAllocateInfo_local = {
		  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		  0,
		  memorySize_local,
		  memoryTypeIndex_local
	};

	//VkDeviceMemory vulkan_memory;
	if (vulkanapi::vkAllocateMemory(vulkan_device, &memoryAllocateInfo_local, 0, &vulkan_memory_local) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem allocating local memory for device.\n";
		return nullptr;

	}


	const VkMemoryAllocateInfo memoryAllocateInfo_image = {
		  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		  0,
		  image_memReqs.size,
		  memoryTypeIndex_local
	};

	//VkDeviceMemory vulkan_memory;
	if (vulkanapi::vkAllocateMemory(vulkan_device, &memoryAllocateInfo_image, 0, &vulkan_memory_image) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem allocating local image memory for device.\n";
		return nullptr;

	}

	vulkanapi::vkBindImageMemory(vulkan_device, vulkan_image, vulkan_memory_image, 0);


	int current_offset = 0;

	//vulkan_bufferCreateInfos.resize(total_vars_count);
	vulkan_buffers.resize(total_vars_count);

	std::vector<VkBufferCreateInfo> vulkan_bufferCreateInfos(total_vars_count);
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(total_vars_count);

	for (uint32_t i = 0; i < external_vars_count; i++)
	{
		//std::vector<VkBufferCreateInfo> vulkan_bufferCreateInfos(0);
		//std::vector<VkBuffer> vulkan_buffers(0);

		vulkan_bufferCreateInfos[i] = {
		  VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		  0,
		  0,
		  vulkan_bufferSizes[i],
		  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		  VK_SHARING_MODE_EXCLUSIVE,
		  1,
		  &vulkan_queueFamilyIndex
		};

		if (vulkanapi::vkCreateBuffer(vulkan_device, &vulkan_bufferCreateInfos[i], 0, &vulkan_buffers[i]) != VK_SUCCESS)
		{
			std::cout << "\nVULKAN ERROR: Problem creating input buffer.\n";
			return nullptr;

		}

		//if (vulkanapi::vkBindBufferMemory(vulkan_device, vulkan_buffers[i], vulkan_memory, 0) != VK_SUCCESS)
		if (vulkanapi::vkBindBufferMemory(vulkan_device, vulkan_buffers[i], vulkan_memory, current_offset) != VK_SUCCESS)
		{
			std::cout << "\nVULKAN ERROR: Problem binding internal vram buffer.\n";
			return nullptr;

		}

		current_offset += vulkan_bufferSizes[i];

		descriptorSetLayoutBindings[i] = { i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0 };
	}


	current_offset = 0;
	for (uint32_t i = external_vars_count; i < total_vars_count; i++)
	{
		//std::vector<VkBufferCreateInfo> vulkan_bufferCreateInfos(0);
		//std::vector<VkBuffer> vulkan_buffers(0);

		vulkan_bufferCreateInfos[i] = {
		  VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		  0,
		  0,
		  vulkan_bufferSizes[i],
		  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		  VK_SHARING_MODE_EXCLUSIVE,
		  1,
		  &vulkan_queueFamilyIndex
		};

		if (vulkanapi::vkCreateBuffer(vulkan_device, &vulkan_bufferCreateInfos[i], 0, &vulkan_buffers[i]) != VK_SUCCESS)
		{
			std::cout << "\nVULKAN ERROR: Problem creating input buffer.\n";
			return nullptr;

		}

		//if (vulkanapi::vkBindBufferMemory(vulkan_device, vulkan_buffers[i], vulkan_memory, 0) != VK_SUCCESS)
		if (vulkanapi::vkBindBufferMemory(vulkan_device, vulkan_buffers[i], vulkan_memory_local, current_offset) != VK_SUCCESS)
		{
			std::cout << "\nVULKAN ERROR: Problem binding internal vram buffer.\n";
			return nullptr;

		}

		current_offset += vulkan_bufferSizes[i];

		descriptorSetLayoutBindings[i] = { i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0 };
	}



	int32_t* payload;



	std::cout << "\nVULKAN: INFO: Creating buffers and binding memory.\n";




	std::cout << "\nVULKAN: INFO: loading and creating compute shader.\n";


	// *** load shader here *** //
	/*
	std::vector<char> ShaderContents;
	int iShaderFileSize;
	if (std::ifstream ShaderFile{ shader_file_name, std::ios::binary | std::ios::ate })
	{
		const size_t FileSize = ShaderFile.tellg();
		ShaderFile.seekg(0);
		ShaderContents.resize(FileSize, '\0');
		ShaderFile.read(ShaderContents.data(), FileSize);
		iShaderFileSize = FileSize;
	}
	else
	{
		std::cout << "\nERROR: VULKAN: Problem loading REQUIRED compiled compute shader file: " << shader_file_name << "\n";
		return nullptr;
	}
	*/

	VkShaderModuleCreateInfo shaderModuleCreateInfo = {
		  VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		  0,
		  0,
		  //sizeof(shader),
		  //shader
		  //ShaderContents.size(),
		//reinterpret_cast<const uint32_t*>(ShaderContents.data())
		shader_file_size,
		reinterpret_cast<const uint32_t*>(shader_file_data)
	};


	//VkShaderModule vulkan_shader_module;

	if (vulkanapi::vkCreateShaderModule(vulkan_device, &shaderModuleCreateInfo, 0, &vulkan_shader_module) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem creating shader module.\n";
		return nullptr;

	}


	std::cout << "\nVULKAN: INFO: creating layouts.\n";


	/*
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[7] = {
	  {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
	  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
	  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
	  {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
	  {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
	  {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
	  {6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0}
	};
	*/


	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
	  VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	  //0, 0, 7, descriptorSetLayoutBindings
	  0, 0, total_vars_count, descriptorSetLayoutBindings.data()
	};


	//VkDescriptorSetLayout vulkan_descriptorSetLayout;
	if (vulkanapi::vkCreateDescriptorSetLayout(vulkan_device, &descriptorSetLayoutCreateInfo, 0, &vulkan_descriptorSetLayout) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem creating descriptor set layout.\n";
		return nullptr;

	}


	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
	  VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	  0,
	  0,
	  1,
	  &vulkan_descriptorSetLayout,
	  0,
	  0
	};

	//VkPipelineLayout vulkan_pipelineLayout;
	if (vulkanapi::vkCreatePipelineLayout(vulkan_device, &pipelineLayoutCreateInfo, 0, &vulkan_pipelineLayout) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem creating pipeline layout.\n";
		return nullptr;

	}

	VkComputePipelineCreateInfo computePipelineCreateInfo = {
	  VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
	  0, 0,
	  {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		0, 0, VK_SHADER_STAGE_COMPUTE_BIT, vulkan_shader_module, "main", 0
	  },
	  vulkan_pipelineLayout, VK_NULL_HANDLE, 0
	};

	//VkPipeline vulkan_pipeline;
	VkResult vkres;
	vkres = vulkanapi::vkCreateComputePipelines(vulkan_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, 0, &vulkan_pipeline);
	//if (vkCreateComputePipelines(vulkan_device, 0, 1, &computePipelineCreateInfo, 0, &vulkan_pipeline) != VK_SUCCESS)
	if (vkres != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem creating compute pipelines.\n";
		std::cout << "\nvkres=" << vkres;
		return nullptr;

	}



	VkDescriptorPoolSize descriptorPoolSize[1] = {
		{
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			total_vars_count	// 7
		}
	};

	//VkDescriptorPoolSize descriptorPoolSize = {
	// VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	 // 1
	//};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
	  VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	  0,
	  0,
	  1,	// 1,
	  1,	// 1,
	  descriptorPoolSize
	};

	//VkDescriptorPool vulkan_descriptorPool;
	if (vulkanapi::vkCreateDescriptorPool(vulkan_device, &descriptorPoolCreateInfo, 0, &vulkan_descriptorPool) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem creating descriptor pool.\n";
		return nullptr;

	}

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
	  VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	  0,
	  vulkan_descriptorPool,
	  1,
	  &vulkan_descriptorSetLayout
	};

	//VkDescriptorSet vulkan_descriptorSet;
	if (vulkanapi::vkAllocateDescriptorSets(vulkan_device, &descriptorSetAllocateInfo, &vulkan_descriptorSet) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem allocating descriptor sets.\n";
		return nullptr;

	}


	std::vector<VkDescriptorBufferInfo> descriptorBufferInfo(total_vars_count);
	std::vector<VkWriteDescriptorSet> writeDescriptorSet(total_vars_count);

	for (uint32_t i = 0; i < total_vars_count; i++)
	{
		descriptorBufferInfo[i] = {
			vulkan_buffers[i],
			0,
			VK_WHOLE_SIZE
		};

		writeDescriptorSet[i] = {
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			0,
			vulkan_descriptorSet,
			i,
			0,
			1,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			0,
			&descriptorBufferInfo[i],
			0
		};

		
	}


	/*
	VkDescriptorBufferInfo scratch_descriptorBufferInfo = {
	  vulkan_scratch_buffer,
	  0,
	  VK_WHOLE_SIZE
	};

	VkDescriptorBufferInfo inout_descriptorBufferInfo = {
	  vulkan_inout_buffer,
	  0,
	  VK_WHOLE_SIZE
	};

	VkDescriptorBufferInfo out_descriptorBufferInfo = {
	  vulkan_out_buffer,
	  0,
	  VK_WHOLE_SIZE
	};

	VkDescriptorBufferInfo incomm_descriptorBufferInfo = {
	  vulkan_incomm_buffer,
	  0,
	  VK_WHOLE_SIZE
	};

	VkDescriptorBufferInfo indata_descriptorBufferInfo = {
	  vulkan_indata_buffer,
	  0,
	  VK_WHOLE_SIZE
	};

	VkDescriptorBufferInfo work_descriptorBufferInfo = {
	  vulkan_work_buffer,
	  0,
	  VK_WHOLE_SIZE
	};

	VkDescriptorBufferInfo staging_descriptorBufferInfo = {
	  vulkan_staging_buffer,
	  0,
	  VK_WHOLE_SIZE
	};
	*/


	std::cout << "\nVULKAN: INFO: creating descriptor set.\n";


	/*
	VkWriteDescriptorSet writeDescriptorSet[7] = {
	  {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		0,
		vulkan_descriptorSet,
		0,
		0,
		1,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		0,
		&scratch_descriptorBufferInfo,
		0
	  },
	  {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		0,
		vulkan_descriptorSet,
		1,
		0,
		1,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		0,
		&inout_descriptorBufferInfo,
		0
	  },
	  {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		0,
		vulkan_descriptorSet,
		2,
		0,
		1,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		0,
		&out_descriptorBufferInfo,
		0
	  },
	  {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		0,
		vulkan_descriptorSet,
		3,
		0,
		1,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		0,
		& incomm_descriptorBufferInfo,
		0
	  },
	  {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		0,
		vulkan_descriptorSet,
		4,
		0,
		1,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		0,
		& indata_descriptorBufferInfo,
		0
	  },
	  {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		0,
		vulkan_descriptorSet,
		5,
		0,
		1,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		0,
		& work_descriptorBufferInfo,
		0
	  },
	  {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		0,
		vulkan_descriptorSet,
		6,
		0,
		1,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		0,
		& staging_descriptorBufferInfo,
		0
	  }
	};
	*/


	//vulkanapi::vkUpdateDescriptorSets(vulkan_device, 7, writeDescriptorSet, 0, 0);
	vulkanapi::vkUpdateDescriptorSets(vulkan_device, total_vars_count, writeDescriptorSet.data(), 0, 0);


	std::cout << "\nVULKAN: INFO: creating swap chain.\n";



	// create swap chain
	vulkan_create_swap_chain();


	



	//VkSubmitInfo submitInfo;
	vulkan_submitInfo = {
	  VK_STRUCTURE_TYPE_SUBMIT_INFO,
	  0,
	  0,
	  0,
	  0,
	  1,
	  &vulkan_commandBuffer,
	  0,
	  0
	};


	//VkQueue queue;
	vulkanapi::vkGetDeviceQueue(vulkan_device, vulkan_queueFamilyIndex, 0, &vulkan_queue);

	// create fence
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vulkanapi::vkCreateFence(vulkan_device, &fenceInfo, nullptr, &vulkan_fence) != VK_SUCCESS) {
		std::cout << "\nVULKAN: ERROR: failed to create fence!";
		return nullptr;
	}


	std::cout << "\nVULKAN: INFO: mapping memory for compute shader\n";

	//if (vkMapMemory(vulkan_device, vulkan_memory, 0, memorySize - work_buffer_size_bytes - image_memReqs.size, 0, (void**)&payload) != VK_SUCCESS)
	if (vulkanapi::vkMapMemory(vulkan_device, vulkan_memory, 0, memorySize, 0, (void**)&payload) != VK_SUCCESS)
	{
		std::cout << "\nVULKAN ERROR: Problem mapping memory for device.\n";
		return nullptr;

	}


	// do a test run to make sure all is good
	((long*)payload)[0] = 0;
	((long*)payload)[1] = 0;
	((long*)payload)[2] = 0;
	((long*)payload)[3] = 0;
	((long*)payload)[4] = 0;
	((long*)payload)[5] = 0;
	((long*)payload)[6] = 0;
	((long*)payload)[7] = 0;
	if (!vulkan_execute_compute_only())
	{
		cout << "\nVULKAN ERROR: PROBLEM: Unable to execute gpu hardware queue (COMPUTE). Minimum required hardware GPU CU Shader Cores is: " << dec << vulkan_num_gpu_x_workgroups;
		cout << "\nVULKAN ERROR: PROBLEM: Hardware GPU unable to handle minimum requirements (COMPUTE ONLY). Reverting back to SOFTWARE renderer.\n";

		//vulkan_SubgroupSize = ((long*)payload)[4];
		vulkan_NumSubgroups = ((long*)payload)[5];
		vulkan_WorkgroupSize = ((long*)payload)[6];
		vulkan_NumWorkgroups = ((long*)payload)[7];

		// show device details
		cout << "\nDevice Name: " << deviceProperties.deviceName;
		cout << "\nDevice Type: " << ((deviceProperties.deviceType == 1) ? "INTEGRATED-GPU" : "DISCRETE-GPU");
		cout << "\nDevice local subgroup size: " << dec << vulkan_SubgroupSize;
		cout << "\nDevice number of local subgroups: " << dec << vulkan_NumSubgroups;
		cout << "\nDevice local workgroup size: " << dec << vulkan_WorkgroupSize;
		cout << "\nDevice number of total workgroups: " << dec << vulkan_NumWorkgroups;

		return nullptr;
	}

	// get device data
	if (!vulkan_wait())
	{
		cout << "\nVULKAN ERROR: PROBLEM: Unable to wait for gpu hardware queue (COMPUTE). Minimum required hardware GPU CU Shader Cores is: " << dec << vulkan_num_gpu_x_workgroups;
		cout << "\nVULKAN ERROR: PROBLEM: Hardware GPU unable to handle minimum requirements (COMPUTE ONLY). Reverting back to SOFTWARE renderer.\n";

		//vulkan_SubgroupSize = ((long*)payload)[4];
		vulkan_NumSubgroups = ((long*)payload)[5];
		vulkan_WorkgroupSize = ((long*)payload)[6];
		vulkan_NumWorkgroups = ((long*)payload)[7];

		// show device details
		cout << "\nDevice Name: " << deviceProperties.deviceName;
		cout << "\nDevice Type: " << ((deviceProperties.deviceType == 1) ? "INTEGRATED-GPU" : "DISCRETE-GPU");
		cout << "\nDevice local subgroup size: " << dec << vulkan_SubgroupSize;
		cout << "\nDevice number of local subgroups: " << dec << vulkan_NumSubgroups;
		cout << "\nDevice local workgroup size: " << dec << vulkan_WorkgroupSize;
		cout << "\nDevice number of total workgroups: " << dec << vulkan_NumWorkgroups;

		return nullptr;
	}

	//vulkan_SubgroupSize = ((long*)payload)[4];
	vulkan_NumSubgroups = ((long*)payload)[5];
	vulkan_WorkgroupSize = ((long*)payload)[6];
	vulkan_NumWorkgroups = ((long*)payload)[7];


	// show device details
	cout << "\n*** VULKAN SETUP SUCCESSFUL ***";
	cout << "\nDevice Name: " << deviceProperties.deviceName;
	cout << "\nDevice Type: " << ((deviceProperties.deviceType == 1) ? "INTEGRATED-GPU" : "DISCRETE-GPU");
	cout << "\nDevice local subgroup size: " << dec << vulkan_SubgroupSize;
	cout << "\nDevice number of local subgroups: " << dec << vulkan_NumSubgroups;
	cout << "\nDevice local workgroup size: " << dec << vulkan_WorkgroupSize;
	cout << "\nDevice number of total workgroups: " << dec << vulkan_NumWorkgroups;


	// delete local vectors
	//std::vector<const char*> required_instance_extensions
	//std::vector<VkExtensionProperties> extensions(extensionCount);
	//std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	//std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
	//std::vector<const char*> required_device_extensions = {
	//std::vector<VkDeviceSize> vulkan_bufferSizes(external_vars_count + internal_vars_count);
	//std::vector<VkBufferCreateInfo> vulkan_bufferCreateInfos(total_vars_count);
	//std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(total_vars_count);
	//std::vector<VkDescriptorBufferInfo> descriptorBufferInfo(total_vars_count);
	//std::vector<VkWriteDescriptorSet> writeDescriptorSet(total_vars_count);
	required_instance_extensions.clear();
	extensions.clear();
	physicalDevices.clear();
	queueFamilyProperties.clear();
	required_device_extensions.clear();
	vulkan_bufferSizes.clear();
	vulkan_bufferCreateInfos.clear();
	descriptorSetLayoutBindings.clear();
	descriptorBufferInfo.clear();
	writeDescriptorSet.clear();


	std::cout << "\n***VULKAN SETUP COMPLETED SUCCESSFULLY***.\n";


	// return mapped memory for device
	return (void*)payload;
}






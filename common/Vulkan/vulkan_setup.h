

#pragma once


#include "WinApiHandler.h"


/// <summary>
/// loads vulkan library
/// returns false when there was a problem
/// return true when successful
/// </summary>
/// <returns>bool</returns>
bool vulkan_load_library();

namespace vulkanapi {

	bool vulkan_load_exported_entry_points();
	bool vulkan_load_global_level_entry_points();
	bool vulkan_load_instance_level_entry_points();
	bool vulkan_load_device_level_entry_points();

}

// must do this before setup - sets size of window to output to
void vulkan_set_screen_size(int screen_size_x, int screen_size_y);


// also must do this before setup or before recreating swap chain if it changes
void vulkan_set_gpu_threads(int num_gpu_threads);


// need to know subgroup size
int vulkan_get_subgroup_size();

// setup vulkan
// shader_file_name - name of the file that contains the computer shader
// inout_buffer_size_bytes - size of ps1/ps2 gpu ram (1MB or 4MB, etc)
// out_buffer_size_bytes - size of ps1/ps2 screen output
// in_comm_buffer_size_bytes - size of input draw commands buffer
// in_data_buffer_size_bytes - size of input data buffer (graphics data)
// work_buffer_size_bytes - size of work buffer for rasterizing primitives (number of input command buffer slots times data needed per slot)
//void* vulkan_setup(HWND wHandle, HINSTANCE wInstance, char* shader_file_data, int shader_file_size, int inout_buffer_size_bytes, int out_buffer_size_bytes, int in_comm_buffer_size_bytes, int in_data_buffer_size_bytes, int work_buffer_size_bytes);
void* vulkan_setup(HWND wHandle, HINSTANCE wInstance, char* shader_file_data, int shader_file_size, long* external_vars_sizes, int external_vars_count, long* internal_vars_sizes, int internal_vars_count );

// create/recreate swap chain
bool vulkan_create_swap_chain();


// execute vulcan job (compute shader run, possibly later update screen, etc)
bool vulkan_execute_compute_only();
bool vulkan_execute();

// wait for last vulkan job to finish
// returns true if successful, returns false on error
bool vulkan_wait();

// wait for fence submitted with last job to signal
// returns true if successful, returns false on error
bool vulkan_wait_fence();


// flush mapped memory writes
void vulkan_flush();

// release vulkan resources
void vulkan_destroy();





#pragma once
#include <string>



struct Shader_data
{
	
	std::string ray_gen_path;
	std::string ray_gen_function;
	std::string closest_hit_path;
	std::string any_hit_path;
	std::string miss_path;
	std::string exception_path;
	
	int min_depth;
	int max_depth;
	float scene_epsilon;
	int sqrt_num_samples;
	int rr_begin_depth;

	Shader_data() {}
};

struct OptiXShader
{
	int index;
	std::string shader_id;
	Shader_data data;

	OptiXShader()
	{

	}
	OptiXShader(std::string shader_id)
		:shader_id(shader_id)
	{
		if (shader_id == "phong")
		{
			index = 0;

			data.ray_gen_path = "pinhole_camera";
			data.ray_gen_function = "pinhole_camera";
			data.closest_hit_path = "closest_hit";
			data.any_hit_path = "any_hit";
			data.miss_path = "miss";
			data.exception_path = "exception";
			
			data.min_depth = 1;
			data.max_depth = 2;
			data.scene_epsilon = 1.e-2f;
			data.sqrt_num_samples = 0;
			data.rr_begin_depth = 0;
		}
		else if (shader_id == "pathtrace_diffuse")
		{
			data.ray_gen_path = "path_trace";
			data.ray_gen_function = "path_trace";
			data.closest_hit_path = "path_trace";
			data.any_hit_path = "path_trace";
			data.miss_path = "path_trace";
			data.exception_path = "path_trace";

			index = 1;
			data.min_depth = 1;
			data.max_depth = 2;
			data.scene_epsilon = 1.5e-3f;
			data.sqrt_num_samples = 1;
			data.rr_begin_depth = 1;
		}
		else if (shader_id == "pathtrace_complete")
		{
			data.ray_gen_path = "path_trace_c";
			data.ray_gen_function = "path_trace";
			data.closest_hit_path = "closest_hit_c";
			data.any_hit_path = "any_hit_c";
			data.miss_path = "miss_c";
			data.exception_path = "exception_c";

			index = 2;
			data.min_depth = 1;
			data.max_depth = 1;
			data.scene_epsilon = 1.e-3f;
			data.sqrt_num_samples = 1;
			data.rr_begin_depth = 1;
		}
	}

};
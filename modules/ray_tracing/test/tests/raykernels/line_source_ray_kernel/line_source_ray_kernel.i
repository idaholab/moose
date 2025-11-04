!include line_source_base.i

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 2 0
                  0.5 0.5 0
                  1 1 0
                  5 5 0
                  2 2 0
                  3 3 0'
  end_points = '3 5 0
                4.5 1.5 0
                2 2 0
                4 1 0
                3 1 0
                3 2 0'
  names = 'constant_source
           pp_source
           function_source
           mixed_source
           data_source
           aux_data_source'
  ray_data_names = 'data'
  ray_aux_data_names = 'aux_data'
  initial_ray_data = '0; 0; 0; 0; 8; 0'
  initial_ray_aux_data = '0; 0; 0; 0; 0; 10'
  execute_on = PRE_KERNELS
[]

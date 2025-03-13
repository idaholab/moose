[Mesh]
    [BatchMeshGeneratorAction]
      [batch1]
        mesh_generator_type = 'ConcentricCircleMeshGenerator'
        multi_batch_params_method = corresponding
        mesh_name_prefix = 'gmg'
        batch_scalar_input_param_names = 'portion'
        batch_scalar_input_param_types = 'ENUM'
        batch_scalar_input_param_values = 'top_right top_left bottom_right bottom_left'
        fixed_scalar_input_param_names = 'has_outer_square preserve_volumes num_sectors'
        fixed_scalar_input_param_types = 'BOOL BOOL UINT'
        fixed_scalar_input_param_values = 'off off 8'
        fixed_vector_input_param_names = 'radii rings'
        fixed_vector_input_param_types = 'REAL UINT'
        fixed_vector_input_param_values = '1.0;1'
      []
    []
    [cmbn]
        type = CombinerGenerator
        inputs = 'gmg_0 gmg_1 gmg_2 gmg_3'
        positions = '0 0 0
                     3 0 0
                     0 3 0
                     3 3 0'
    []
[]

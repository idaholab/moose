[Mesh]
    [BatchMeshGeneratorAction]
      [batch1]
        mesh_generator_type = 'ConcentricCircleMeshGenerator'
        multi_batch_params_method = corresponding
        mesh_name_prefix = 'gmg'
        batch_vector_input_param_names = 'radii rings'
        batch_vector_input_param_types = 'REAL UINT'
        batch_vector_input_param_values = '0.8;1.0;1.2;1.4| 1;2;1;2'
        fixed_scalar_input_param_names = 'has_outer_square preserve_volumes num_sectors portion'
        fixed_scalar_input_param_types = 'BOOL BOOL UINT ENUM'
        fixed_scalar_input_param_values = 'off off 8 top_right'
      []
    []
    [cmbn]
        type = CombinerGenerator
        inputs = 'gmg_3 gmg_2 gmg_1 gmg_0'
        positions = '3 3 0
                     0 3 0
                     3 0 0
                     0 0 0'
    []
[]

[Mesh]
    [BatchMeshGeneratorAction]
      [batch1]
        mesh_generator_type = 'ConcentricCircleMeshGenerator'
        multi_batch_params_method = corresponding
        mesh_name_prefix = 'gmg'
        batch_vector_input_param_names = 'radii rings'
        batch_vector_input_param_types = 'REAL UINT'
        batch_vector_input_param_values = '0.5;0.8;1.2;1.5|1;2;3;4'
        batch_scalar_input_param_names = 'num_sectors'
        batch_scalar_input_param_types = 'UINT'
        batch_scalar_input_param_values = '6 8 10 12'
        fixed_scalar_input_param_names = 'portion has_outer_square preserve_volumes'
        fixed_scalar_input_param_types = 'ENUM BOOL BOOL'
        fixed_scalar_input_param_values = 'left_half off off'
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

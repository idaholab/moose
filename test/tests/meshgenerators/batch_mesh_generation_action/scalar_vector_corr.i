[Mesh]
    [BatchMeshGeneratorAction]
      [batch1]
        mesh_generator_type = 'GeneratedMeshGenerator'
        multi_batch_params_method = corresponding
        mesh_name_prefix = 'gmg'
        batch_scalar_input_param_names = 'nx'
        batch_scalar_input_param_types = 'UINT'
        batch_scalar_input_param_values = '2 3 4 5'
        batch_vector_input_param_names = 'subdomain_ids'
        batch_vector_input_param_types = 'USHORT'
        batch_vector_input_param_values = '1;2;3;4'
        fixed_scalar_input_param_names = 'dim'
        fixed_scalar_input_param_types = 'ENUM'
        fixed_scalar_input_param_values = '2'
      []
    []
    [cmbn]
        type = CombinerGenerator
        inputs = 'gmg_3 gmg_2 gmg_1 gmg_0'
        positions = '2 2 0
                     0 2 0
                     2 0 0
                     0 0 0'
    []
[]

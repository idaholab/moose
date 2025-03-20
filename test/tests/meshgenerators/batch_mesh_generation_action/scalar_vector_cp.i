[Mesh]
    [BatchMeshGeneratorAction]
      [batch1]
        mesh_generator_type = 'GeneratedMeshGenerator'
        multi_batch_params_method = cartesian_product
        mesh_name_prefix = 'gmg'
        batch_scalar_input_param_names = 'nx'
        batch_scalar_input_param_types = 'UINT'
        batch_scalar_input_param_values = '2 3'
        batch_vector_input_param_names = 'subdomain_ids'
        batch_vector_input_param_types = 'USHORT'
        batch_vector_input_param_values = '1;2'
        fixed_scalar_input_param_names = 'dim'
        fixed_scalar_input_param_types = 'ENUM'
        fixed_scalar_input_param_values = '2'
        use_decomposed_index = true
      []
    []
    [cmbn]
        type = CombinerGenerator
        inputs = 'gmg_1_1 gmg_0_1 gmg_1_0 gmg_0_0'
        positions = '2 2 0
                     0 2 0
                     2 0 0
                     0 0 0'
    []
[]

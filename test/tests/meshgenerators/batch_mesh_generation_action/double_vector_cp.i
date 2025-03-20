[Mesh]
  [BatchMeshGeneratorAction]
    [batch1]
      mesh_generator_type = 'ConcentricCircleMeshGenerator'
      multi_batch_params_method = cartesian_product
      mesh_name_prefix = 'gmg'
      batch_vector_input_param_names = 'radii rings'
      batch_vector_input_param_types = 'REAL UINT'
      batch_vector_input_param_values = '0.8;1.0| 1 1;2 1'
      fixed_scalar_input_param_names = 'has_outer_square preserve_volumes num_sectors portion pitch'
      fixed_scalar_input_param_types = 'BOOL BOOL UINT ENUM REAL'
      fixed_scalar_input_param_values = 'on off 4 top_right 2.1'
      use_decomposed_index = true
    []
  []
  [cmbn]
      type = CombinerGenerator
      inputs = 'gmg_1_1 gmg_0_1 gmg_1_0 gmg_0_0'
      positions = '3 3 0
                   0 3 0
                   3 0 0
                   0 0 0'
  []
[]

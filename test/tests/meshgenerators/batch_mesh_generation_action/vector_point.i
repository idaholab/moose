[Mesh]
  [BatchMeshGeneratorAction]
    [batch1]
      mesh_generator_type = 'ElementGenerator'
      mesh_name_prefix = 'eg'
      batch_vector_input_param_names = 'nodal_positions'
      batch_vector_input_param_types = 'POINT'
      batch_vector_input_param_values = '0 0 0 1 0 0 1 1 0;
                                         2 0 0 3 0 0 3 1 0;
                                         1 2 0 2 2 0 2 3 0'
      fixed_scalar_input_param_names = 'elem_type'
      fixed_scalar_input_param_types = 'ENUM'
      fixed_scalar_input_param_values = 'TRI3'
      fixed_vector_input_param_names = 'element_connectivity'
      fixed_vector_input_param_types = 'DOFIDTYPE'
      fixed_vector_input_param_values = '0 1 2'
    []
  []
  [cmbn]
      type = CombinerGenerator
      inputs = 'eg_0 eg_1 eg_2'
  []
[]

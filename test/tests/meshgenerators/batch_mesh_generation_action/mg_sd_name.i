[Mesh]
  [BatchMeshGeneratorAction]
    [batch1]
      mesh_generator_type = 'RenameBlockGenerator'
      mesh_name_prefix = 'rbg'
      fixed_scalar_input_param_names = 'input'
      fixed_scalar_input_param_types = 'MGNAME'
      fixed_scalar_input_param_values = 'gmg'
      fixed_vector_input_param_names = 'old_block'
      fixed_vector_input_param_types = 'SDNAME'
      fixed_vector_input_param_values = '0'
      batch_vector_input_param_names = 'new_block'
      batch_vector_input_param_types = 'SDNAME'
      batch_vector_input_param_values = '1;2;3;4'
    []
  []
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
  [cmbn]
    type = CombinerGenerator
    inputs = 'rbg_0 rbg_1 rbg_2 rbg_3'
    positions = '0 0 0
                 0 0 1
                 0 0 2
                 0 0 3'
  []
[]

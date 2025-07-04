[Mesh]
  [BatchMeshGeneratorAction]
    [batch1]
      mesh_generator_type = 'TransformGenerator'
      mesh_name_prefix = 'rotate'
      batch_scalar_input_param_names = 'input'
      batch_scalar_input_param_types = 'MGNAME'
      batch_scalar_input_param_values = 'gmg_0 gmg_1'
      fixed_scalar_input_param_names = 'transform vector_value'
      fixed_scalar_input_param_types = 'ENUM REALVECTORVALUE'
      fixed_scalar_input_param_values = 'ROTATE 45 0 0'
    []
  []
  [gmg_0]
      type = GeneratedMeshGenerator
      dim = 2
      subdomain_ids = 1
  []
  [gmg_1]
      type = GeneratedMeshGenerator
      dim = 2
      subdomain_ids = 2
  []
  [cmbn]
      type = CombinerGenerator
      inputs = 'rotate_0 rotate_1'
      positions = '0 0 0
                   2 0 0'
  []
[]

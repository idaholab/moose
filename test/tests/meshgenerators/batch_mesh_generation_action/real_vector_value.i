[Mesh]
  [BatchMeshGeneratorAction]
    [batch1]
      mesh_generator_type = 'TransformGenerator'
      mesh_name_prefix = 'translate'
      batch_scalar_input_param_names = 'vector_value'
      batch_scalar_input_param_types = 'REALVECTORVALUE'
      batch_scalar_input_param_values = '0 0 0 2 0 0 0 2 0 2 2 0'
      fixed_scalar_input_param_names = 'input transform'
      fixed_scalar_input_param_types = 'MGNAME ENUM'
      fixed_scalar_input_param_values = 'gmg TRANSLATE'
    []
  []
  [gmg]
      type = GeneratedMeshGenerator
      dim = 2
      subdomain_ids = 1
  []
  [cmbn]
      type = CombinerGenerator
      inputs = 'translate_0 translate_1 translate_2 translate_3'
  []
[]

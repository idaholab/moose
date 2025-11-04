[Mesh]
  [BatchMeshGeneratorAction]
    [batch1]
      mesh_generator_type = 'FileMeshGenerator'
      mesh_name_prefix = 'fmg'
      batch_scalar_input_param_names = 'file'
      batch_scalar_input_param_types = 'MFNAME'
      batch_scalar_input_param_values = 'gold/element_generator_in.e gold/element_generator_ss_in.e'
    []
  []
  [cmbn]
    type = CombinerGenerator
    inputs = 'fmg_0 fmg_1'
    positions = '0 0 0
                 0 0 2'
  []
[]

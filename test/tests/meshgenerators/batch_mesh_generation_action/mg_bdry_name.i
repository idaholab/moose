[Mesh]
  [BatchMeshGeneratorAction]
    [batch1]
      mesh_generator_type = 'RenameBoundaryGenerator'
      mesh_name_prefix = 'rbg'
      fixed_scalar_input_param_names = 'input'
      fixed_scalar_input_param_types = 'MGNAME'
      fixed_scalar_input_param_values = 'gmg'
      fixed_vector_input_param_names = 'old_boundary'
      fixed_vector_input_param_types = 'BDRYNAME'
      fixed_vector_input_param_values = 'bottom right top left'
      batch_vector_input_param_names = 'new_boundary'
      batch_vector_input_param_types = 'BDRYNAME'
      batch_vector_input_param_values = 'b0 r0 t0 l0;b1 r1 t1 l1;b2 r2 t2 l2;b3 r3 t3 l3'
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
    avoid_merging_boundaries = true
    positions = '0 0 0
                 0 0 1
                 0 0 2
                 0 0 3'
  []
[]

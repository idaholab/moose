# This tests the VectorPostprocessorDifference post-processor, which takes two
# vector post-processors and computes a difference measure between them

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
  xmin = 0
  xmax = 2
[]

[Functions]
  # Sampled values will be [2, 2, 2]
  [./a_fn]
    type = ConstantFunction
    value = 2
  [../]
  # Sampled values will be [0, 1, 2]
  [./b_fn]
    type = ParsedFunction
    value = 'x'
  [../]
[]

[VectorPostprocessors]
  [./a_vpp]
    type = LineFunctionSampler
    functions = 'a_fn'
    num_points = 3
    start_point = '0 0 0'
    end_point = '2 0 0'
    sort_by = x
    execute_on = 'initial'
  [../]
  [./b_vpp]
    type = LineFunctionSampler
    functions = 'b_fn'
    num_points = 3
    start_point = '0 0 0'
    end_point = '2 0 0'
    sort_by = x
    execute_on = 'initial'
  [../]
[]

[Postprocessors]
  [./vpp_difference]
    type = VectorPostprocessorDifferenceMeasure
    vectorpostprocessor_a = a_vpp
    vectorpostprocessor_b = b_vpp
    vector_name_a = a_fn
    vector_name_b = b_fn
    difference_type = l2
    execute_on = 'initial'
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = l2
  csv = true
  show = 'vpp_difference'
  execute_on = 'initial'
[]

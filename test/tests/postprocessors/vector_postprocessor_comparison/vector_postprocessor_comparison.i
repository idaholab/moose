# This tests the VectorPostprocessorComparison post-processor, which takes two
# vector post-processors and compares them.

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
    expression = 'x'
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
  [./vpp_comparison]
    type = VectorPostprocessorComparison
    vectorpostprocessor_a = a_vpp
    vectorpostprocessor_b = b_vpp
    vector_name_a = a_fn
    vector_name_b = b_fn
    comparison_type = greater_than_equals
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
  file_base = greater_than_equals
  csv = true
  show = 'vpp_comparison'
  execute_on = 'initial'
[]

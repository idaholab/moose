# This tests the PostprocessorComparison post-processor, which compares two
# post-processors.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmin = 0
  xmax = 1
[]

[Postprocessors]
  [./pp_to_compare]
    type = LinearCombinationPostprocessor
    pp_names = ''
    pp_coefs = ''
    b = 1
  [../]
  [./pp_comparison]
    type = PostprocessorComparison
    value_a = pp_to_compare
    value_b = 2
    comparison_type = greater_than
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
  file_base = greater_than
  csv = true
  show = 'pp_comparison'
  execute_on = 'initial'
[]

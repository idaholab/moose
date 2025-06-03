[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    components = 3
    initial_condition = '1 2 3'
    array_var_component_names = 'x y z'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[Variables]
  [u]
    initial_condition = 1
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [move]
    type = MoveNodesByParsedExpression
    block = 0
    displacement_x = '0.1*sin(pi*y)'
    displacement_y = '0'
    constant_names = 'pi'
    constant_expressions = '3.14159265358979'
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  exodus = true
[]

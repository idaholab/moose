# Test vec() operator in energy expressions
#
# This test checks if vec() works in differentiation

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[AutomaticWeakForm]
  energy_type = EXPRESSION

  # Test 1: Direct use of vec() in dot product
  # This creates a vector from components and takes dot product
  energy_expression = 'dot(vec(u, v), vec(u, v))'

  variables = 'u v'
  use_automatic_differentiation = true
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = false
  console = true
[]
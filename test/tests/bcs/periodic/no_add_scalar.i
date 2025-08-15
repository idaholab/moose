# Test to make sure that periodic boundaries
# are not applied to scalar variables.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables/scalar]
  family = SCALAR
[]

[BCs/Periodic/scalar]
  variable = scalar
  auto_direction = x
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

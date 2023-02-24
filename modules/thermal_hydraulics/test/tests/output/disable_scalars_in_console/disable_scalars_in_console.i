[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [var]
    family = SCALAR
    order = FIRST
    initial_condition = 5.0
  []
[]

# The components block ultimately triggers THMSetupOutputAction
[Components]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = ADDiffusion
    variable = u
  []

  [reaction]
    type = ADReaction
    variable = u
  []

  [force]
    type = ADBodyForce
    variable = u
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    boundary = left
    variable = u
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]

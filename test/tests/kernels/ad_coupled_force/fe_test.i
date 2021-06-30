[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [diff_u]
    type = ADDiffusion
    variable = u
  []
  [force_u]
    type = ADCoupledForce
    variable = u
    v = v
  []

  [diff_v]
    type = ADDiffusion
    variable = v
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []

  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 5
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 3
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymax = 1
    nx = 10
    ny = 10
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 0
  []
[]

[Postprocessors]
  [unorm]
    type = ElementL2Norm
    variable = u
  []
[]

[Executioner]
  type = TestSteady
  solve_type = 'NEWTON'
  set_control = BCs/right/value
  control_value = 2
[]

[Outputs]
  exodus = true
[]

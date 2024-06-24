[Mesh]
  type = GeneratedMesh

  dim = 2

  xmin = 0
  xmax = 1

  ymin = 0
  ymax = 1

  nx = 10
  ny = 10
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []

  [v]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [parsed]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []

  [diff_v]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = top
    value = 0
  []

  [right_u]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 1
  []

  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []

  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[AuxKernels]
  [set_parsed]
    type = ParsedAux
    variable = parsed
    functor_names = 'u v'
    functor_symbols = 'u v'
    expression = '(u-0.5)^3*v'
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]

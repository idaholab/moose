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
    boundary = 2
    value = 0
  []

  [right_u]
    type = DirichletBC
    variable = u
    boundary = 0
    value = 1
  []

  [left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 0
  []

  [right_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 1
  []
[]

[Bounds]
  [u_bounds]
    type = ParsedAux
    variable = parsed
    coupled_variables = 'u v'
    expression = '(u-0.5)^3*v'
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
  exodus = true
[]

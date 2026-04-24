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
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [diff_u]
    type = KokkosDiffusion
    variable = u
  []

  [diff_v]
    type = KokkosDiffusion
    variable = v
  []
[]

[BCs]
  [left_u]
    type = KokkosDirichletBC
    variable = u
    boundary = top
    value = 0
  []

  [right_u]
    type = KokkosDirichletBC
    variable = u
    boundary = bottom
    value = 1
  []

  [left_v]
    type = KokkosDirichletBC
    variable = v
    boundary = left
    value = 0
  []

  [right_v]
    type = KokkosDirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Materials]
  [u]
    type = KokkosParsedMaterial
    property_name = 'u_mat'
    expression = 'u'
    coupled_variables = 'u'
  []
  [v]
    type = KokkosParsedMaterial
    property_name = 'v_mat'
    expression = 'v'
    coupled_variables = 'v'
  []
[]

[AuxKernels]
  [set_parsed]
    type = KokkosParsedAux
    variable = parsed
    material_property_names = 'u_mat v_mat'
    expression = '(u_mat-0.5)^3*v_mat'
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
  hide = 'u v'
[]

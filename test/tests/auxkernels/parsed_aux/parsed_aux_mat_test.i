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

[Materials]
  [u]
    type = ParsedMaterial
    property_name = 'u_mat'
    expression = 'u'
    coupled_variables = 'u'
  []
  [v]
    type = ParsedMaterial
    property_name = 'v_mat'
    expression = 'v'
    coupled_variables = 'v'
  []
[]

[AuxKernels]
  [set_parsed]
    type = ParsedAux
    variable = parsed
    material_properties = 'u_mat v_mat'
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

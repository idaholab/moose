[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 10
    xmax = 10
    ymax = 3
    elem_type = TRI3
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [left_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0
  []

  inactive = 'right_dirichlet'
  [right_neumann]
    type = FunctionNeumannBC
    variable = disp_x
    function = t
    boundary = right
  []
  [right_dirichlet]
    type = FunctionDirichletBC
    variable = disp_x
    function = t/10
    boundary = right
  []
[]

[UserObjects]
  [uel]
    type = AbaqusUserElement
    variables = 'disp_x disp_y'
    plugin = ../../plugins/elastic_uel_tri
    use_displaced_mesh = false
    num_state_vars = 8
    constant_properties = '100 0.3' # E nu
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 15
[]

[Postprocessors]
  [delta_l]
    type = SideAverageValue
    variable = disp_x
    boundary = right
  []
  [V]
    type = ElementIntegralMaterialProperty
    mat_prop = 1
  []
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
    xmax = 5
    ymax = 1.5
    elem_type = TRI3
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[ICs]
  [x]
    type = FunctionIC
    variable = disp_x
    function = x/100+0.0001
  []
  [y]
    type = FunctionIC
    variable = disp_y
    function = 0.00077
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
  [right]
    type = FunctionNeumannBC
    variable = disp_x
    function = t
    boundary = right
  []
[]

[UserObjects]
  [uel]
    type = AbaqusUserElement
    variables = 'disp_x disp_y'
    plugin = ../../plugins/debug_uel_tri
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
  num_steps = 10
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
[]

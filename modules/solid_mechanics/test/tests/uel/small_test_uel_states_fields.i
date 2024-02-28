[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 10
    # nx = 10
    # ny = 3
    xmax = 10
    ymax = 3
    elem_type = TRI3
  []
  [pin]
    type = ExtraNodesetGenerator
    nodes = 106
    new_boundary = pin
    input = gen
  []
  displacements = 'disp_x disp_y'
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [temperature]
    initial_condition = 1500
  []
  [voltage]
    initial_condition = 210
  []
[]

[AuxKernels]
  [temperature]
    type = FunctionAux
    function = temperature_function
    variable = temperature
  []
  [voltage]
    type = FunctionAux
    function = voltage_function
    variable = voltage
  []
[]

[Functions]
  [voltage_function]
    type = PiecewiseLinear
    x = '0 15'
    y = '210 450'
  []
  [temperature_function]
    type = PiecewiseLinear
    x = '0 15'
    y = '1500 800'
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
    boundary = pin
    value = 0
  []

  inactive = 'right_dirichlet'
  # inactive = 'right_neumann'
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
    plugin = ../../../examples/uel_tri_states_tests/uel
    use_displaced_mesh = false
    num_state_vars = 8
    constant_properties = '100 0.3' # E nu
    external_fields = 'temperature voltage'
    extra_vector_tags = 'kernel_residual'
  []
[]

[Problem]
  kernel_coverage_check = false
  extra_tag_vectors = 'kernel_residual'
[]

[AuxVariables]
  [res_x]
  []
  [res_y]
  []
[]

[AuxKernels]
  [res_x]
    type = TagVectorAux
    variable = res_x
    v = disp_x
    vector_tag = kernel_residual
  []
  [res_y]
    type = TagVectorAux
    variable = res_y
    v = disp_y
    vector_tag = kernel_residual
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 1
  num_steps = 15
[]

[Postprocessors]
  [delta_l]
    type = SideAverageValue
    variable = disp_x
    boundary = right
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [V]
    type = ElementIntegralMaterialProperty
    mat_prop = 1
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

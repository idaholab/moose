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
    plugin = ../../../examples/uel_tri_tests/uel
    use_displaced_mesh = false
    num_state_vars = 6
    constant_properties = '100 0.3' # E nu
    extra_residual_tags = 'kernel_residual'
  []
[]

[Problem]
  kernel_coverage_check = false
  extra_tag_residuals = 'kernel_residual'
[]

[AuxVariables]
  [res_x]
  []
  [res_y]
  []
[]

[AuxKernels]
  [res_x]
    type = TagResidualAux
    variable = res_x
    v = disp_x
    residual_tag = kernel_residual
  []
  [res_y]
    type = TagResidualAux
    variable = res_y
    v = disp_y
    residual_tag = kernel_residual
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

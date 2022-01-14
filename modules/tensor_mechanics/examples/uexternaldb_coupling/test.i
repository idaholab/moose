[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
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
    type = FunctionDirichletBC
    boundary = right
    variable = disp_x
    function = t
  []
[]

[UserObjects]
  [uexternaldb]
    type = AbaqusUExternalDB
    plugin = umat
    execute_on = 'INITIAL TIMESTEP_END FINAL'
  []
[]

[Materials]
  [umat]
    type = AbaqusUMATStress
    constant_properties = '1 0.3'
    num_state_vars = 0
    plugin = umat
    # plugin = ../../test/plugins/elastic
    use_one_based_indexing = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  dt = 0.01
  num_steps = 10
[]

[Outputs]
  print_linear_residuals = false
[]

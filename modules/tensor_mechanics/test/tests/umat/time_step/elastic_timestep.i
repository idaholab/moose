# Testing the UMAT Interface - linear elastic model using the large strain formulation.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
  []
[]

[Functions]
  [top_pull]
    type = ParsedFunction
    expression = t/100
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
  []
[]

[BCs]
  [y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = top_pull
  []
  [x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0.0
  []
[]

[Materials]
  [umat]
    type = AbaqusUMATStress
    constant_properties = '1000 0.3'
    plugin = '../../../plugins/elastic_timestep'
    num_state_vars = 0
    use_one_based_indexing = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9
  start_time = 0.0
  end_time = 30

  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 30
    iteration_window = 9
    growth_factor = 2.0
    cutback_factor = 1.0
    timestep_limiting_postprocessor = matl_ts_min
    dt = 1.0
  []
[]

[UserObjects]
  [time_step_size]
    type = TimestepSize
    execute_on = 'INITIAL LINEAR'
  []
  [terminator_umat]
    type = Terminator
    expression = 'time_step_size > matl_ts_min'
    fail_mode = SOFT
    execute_on = 'FINAL'
  []
[]

[Postprocessors]
  [matl_ts_min]
    type = MaterialTimeStepPostprocessor
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = true
[]

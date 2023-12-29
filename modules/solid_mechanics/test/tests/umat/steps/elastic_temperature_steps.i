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
  [top_pull_step2]
    type = ParsedFunction
    expression = (t-5.0)/20
  []
  # Forced evolution of temperature
  [temperature_load]
    type = ParsedFunction
    expression = '273'
  []
[]

[AuxVariables]
  [temperature]
  []
[]

[AuxKernels]
  [temperature_function]
    type = FunctionAux
    variable = temperature
    function = temperature_load
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'stress_yy'
  []
[]

[BCs]
  [y_step1]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0.0
  []
  [y_pull_function_step2]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = top_pull_step2
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

[Controls]
  [step1]
    type = TimePeriod
    enable_objects = 'BCs::y_step1'
    disable_objects = 'BCs::y_pull_function_step2'
    start_time = '0'
    end_time = '5'
  []
  [step2]
    type = TimePeriod
    enable_objects = 'BCs::y_pull_function_step2'
    disable_objects = 'BCs::y_step1'
    start_time = '5'
    end_time = '10'
  []
[]

[Materials]
  # This input file is used to compare the MOOSE and UMAT models, activating
  # specific ones with cli variable_names.

  # 1. Active for umat calculation
  [umat]
    type = AbaqusUMATStress
    constant_properties = '1000 0.3'
    plugin = '../../../plugins/elastic_temperature'
    num_state_vars = 0
    temperature = temperature
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
  num_steps = 10
  dt = 1.0
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

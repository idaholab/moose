rho = 1
l = 1
U = 1
gamma = 1e4
degree = 2
alpha = '${fparse 10 * degree^2}'

[Problem]
  type = NavierStokesProblem
  extra_tag_matrices = 'mass'
  mass_matrix = 'mass'
  set_schur_pre = A11_AND_MASS
[]

[Variables]
  [vel_x]
    family = L2_LAGRANGE
    order = SECOND
  []
  [vel_y]
    family = L2_LAGRANGE
    order = SECOND
  []
  [vel_z]
    family = L2_LAGRANGE
    order = SECOND
  []
  [pressure]
    family = L2_LAGRANGE
    order = FIRST
  []
  [vel_bar_x]
    family = LAGRANGE
    order = SECOND
  []
  [vel_bar_y]
    family = LAGRANGE
    order = SECOND
  []
  [vel_bar_z]
    family = LAGRANGE
    order = SECOND
  []
  [pressure_bar]
    family = SIDE_HIERARCHIC
    order = SECOND
  []
[]

[HDGKernels]
  [momentum_x_convection]
    type = AdvectionIPHDGKernel
    variable = vel_x
    face_variable = vel_bar_x
    velocity = 'velocity'
    coeff = ${rho}
  []
  [momentum_x_diffusion]
    type = NavierStokesStressIPHDGKernel
    variable = vel_x
    face_variable = vel_bar_x
    diffusivity = 'mu'
    alpha = ${alpha}
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    component = 0
  []
  [momentum_y_convection]
    type = AdvectionIPHDGKernel
    variable = vel_y
    face_variable = vel_bar_y
    velocity = 'velocity'
    coeff = ${rho}
  []
  [momentum_y_diffusion]
    type = NavierStokesStressIPHDGKernel
    variable = vel_y
    face_variable = vel_bar_y
    diffusivity = 'mu'
    alpha = ${alpha}
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    component = 1
  []
  [momentum_z_convection]
    type = AdvectionIPHDGKernel
    variable = vel_z
    face_variable = vel_bar_z
    velocity = 'velocity'
    coeff = ${rho}
  []
  [momentum_z_diffusion]
    type = NavierStokesStressIPHDGKernel
    variable = vel_z
    face_variable = vel_bar_z
    diffusivity = 'mu'
    alpha = ${alpha}
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    component = 2
  []

  [pressure]
    type = MassContinuityIPHDGKernel
    variable = pressure
    face_variable = pressure_bar
    interior_velocity_vars = 'vel_x vel_y vel_z'
    face_velocity_functors = 'vel_bar_x vel_bar_y vel_bar_z'
  []

  [u_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    w = vel_z
    component = 0
    gamma = ${gamma}
    face_velocity = face_velocity
  []
  [v_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    w = vel_z
    component = 1
    gamma = ${gamma}
    face_velocity = face_velocity
  []
  [w_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_z
    face_variable = vel_bar_z
    u = vel_x
    v = vel_y
    w = vel_z
    component = 2
    gamma = ${gamma}
    face_velocity = face_velocity
  []
  [pb_mass]
    type = MassMatrixHDG
    variable = pressure_bar
    matrix_tags = 'mass'
    density = '${fparse -1/gamma}'
  []
[]

[Functions]
  [reynolds]
    type = ParsedFunction
    expression = 't'
  []
[]

[FunctorMaterials]
  [face_velocity]
    type = ADGenericVectorFunctorMaterial
    prop_names = face_velocity
    prop_values = 'vel_bar_x vel_bar_y vel_bar_z'
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho'
    prop_values = '${rho}'
  []
  [vel]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = 'velocity'
    u = vel_x
    v = vel_y
    w = vel_z
  []
  [mu]
    type = ADParsedMaterial
    functor_names = 'reynolds'
    functor_symbols = 'reynolds'
    property_name = 'mu'
    expression = '${U} * ${l} / reynolds'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options = '-nl0_condensed_ksp_view' # -mat_strumpack_verbose'
  petsc_options_iname = '-ksp_type' # -mat_strumpack_compression_rel_tol'
  petsc_options_value = 'preonly' #   1e-8'
  [TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '1 10 50 100 150 200 250 350 400 600 800 1000 1200 1400 1600 1800 2000 2200 2400 2600 2800 3000 3200 3400 3600 3800 4000 4200 4400 4600 4800 5000 5200 5400 5600 5800 6000 6200 6400 6600 6800 7000 7200 7400 7600 7800 8000 8200 8400 8600 8800 9000 9200 9400 9600 9800 10000'
    use_last_t_for_end_time = true
  []
  abort_on_solve_fail = false
  nl_max_its = 10
  line_search = basic
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-7
  nl_rel_step_tol = 1e-10
[]

[Outputs]
  print_linear_residuals = 'false'
  exodus = false
  checkpoint = true
  perf_graph = true
[]

[Postprocessors]
  [Re]
    type = FunctionValuePostprocessor
    function = 'reynolds'
  []
  [pressure_average]
    type = ElementAverageValue
    variable = pressure
  []
[]

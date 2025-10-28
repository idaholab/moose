gamma = 1e4
degree = 2
alpha = '${fparse 10 * degree^2}'
rho = 1

[Mesh]
  [file]
    type = FileMeshGenerator
    file = coarse06.msh
  []
  second_order = true
[]

[Problem]
  type = NavierStokesProblem
  extra_tag_matrices = 'mass'
  mass_matrix = 'mass'
  use_pressure_mass_matrix = true
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
  [pressure_bar]
    family = LAGRANGE
    order = SECOND
  []
[]

[HDGKernels]
  [momentum_x_advection]
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
  [momentum_y_advection]
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
  [pressure]
    type = MassContinuityIPHDGKernel
    variable = pressure
    face_variable = pressure_bar
    interior_velocity_vars = 'vel_x vel_y'
    face_velocity_functors = 'vel_bar_x vel_bar_y'
  []

  [u_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_x
    face_variable = vel_bar_x
    face_velocity = face_velocity
    u = vel_x
    v = vel_y
    component = 0
    gamma = ${gamma}
  []
  [v_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_y
    face_variable = vel_bar_y
    face_velocity = face_velocity
    u = vel_x
    v = vel_y
    component = 1
    gamma = ${gamma}
  []
  [pb_mass]
    type = MassMatrixHDG
    variable = pressure_bar
    matrix_tags = 'mass'
    density = '${fparse -1/gamma}'
  []
[]

[BCs]
  #
  # inlet
  #
  [momentum_x_advection_inlet]
    type = AdvectionIPHDGDirichletBC
    boundary = '1'
    face_variable = vel_bar_x
    functor = u_inlet
    variable = vel_x
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_diffusion_inlet]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = '1'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = 'u_inlet'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_y_diffusion_inlet]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = '1'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = 0
    diffusivity = 'mu'
    component = 1
  []
  [mass_inlet]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = '1'
    face_velocity_functors = 'u_inlet 0'
    interior_velocity_vars = 'vel_x vel_y'
  []

  #
  # walls
  #
  [momentum_x_diffusion_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = '2'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_y_diffusion_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = '2'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = 0
    diffusivity = 'mu'
    component = 1
  []
  [mass_walls]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = '2'
    face_velocity_functors = '0 0'
    interior_velocity_vars = 'vel_x vel_y'
  []

  #
  # Neumann
  #
  [momentum_x_advection_neumann]
    type = AdvectionIPHDGOutflowBC
    boundary = '3'
    constrain_lm = false
    face_variable = vel_bar_x
    variable = vel_x
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_y_advection_neumann]
    type = AdvectionIPHDGOutflowBC
    boundary = '3'
    constrain_lm = false
    face_variable = vel_bar_y
    variable = vel_y
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_diffusion_neumann]
    type = NavierStokesStressIPHDGPrescribedFluxBC
    boundary = '3'
    component = 0
    diffusivity = 'mu'
    face_variable = vel_bar_x
    prescribed_normal_flux = 0
    pressure_face_variable = pressure_bar
    pressure_variable = pressure
    variable = vel_x
    alpha = ${alpha}
  []
  [momentum_y_diffusion_neumann]
    type = NavierStokesStressIPHDGPrescribedFluxBC
    boundary = '3'
    component = 1
    diffusivity = 'mu'
    face_variable = vel_bar_y
    prescribed_normal_flux = 0
    pressure_face_variable = pressure_bar
    pressure_variable = pressure
    variable = vel_y
    alpha = ${alpha}
  []
  [mass_neumann]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = '3'
    face_velocity_functors = 'vel_bar_x vel_bar_y'
    interior_velocity_vars = 'vel_x vel_y'
  []

  [pb_mass]
    type = MassMatrixIntegratedBC
    variable = pressure_bar
    matrix_tags = 'mass'
    boundary = '1 2 3'
    density = '${fparse -1/gamma}'
  []

  [u_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    component = 0
    boundary = '2'
    gamma = ${gamma}
    face_velocity = vel_walls
    dirichlet_boundary = true
  []
  [v_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    component = 1
    boundary = '2'
    gamma = ${gamma}
    face_velocity = vel_walls
    dirichlet_boundary = true
  []
  [u_jump_inlet]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    component = 0
    boundary = '1'
    gamma = ${gamma}
    face_velocity = vel_inlet
    dirichlet_boundary = true
  []
  [v_jump_inlet]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    component = 1
    boundary = '1'
    gamma = ${gamma}
    face_velocity = vel_inlet
    dirichlet_boundary = true
  []
  [u_jump_outlet]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    component = 0
    boundary = '3'
    gamma = ${gamma}
    face_velocity = face_velocity
    dirichlet_boundary = false
  []
  [v_jump_outlet]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    component = 1
    boundary = '3'
    gamma = ${gamma}
    face_velocity = face_velocity
    dirichlet_boundary = false
  []
[]

[Materials]
  [vel]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = 'velocity'
    u = vel_x
    v = vel_y
  []
  [mu]
    type = ADParsedMaterial
    functor_names = 'reynolds'
    functor_symbols = 'reynolds'
    property_name = 'mu'
    expression = '1 / reynolds'
  []
[]

[Functions]
  [u_inlet]
    type = ParsedFunction
    expression = '4*(2-y)*(y-1)'
  []
  [reynolds]
    type = ParsedFunction
    expression = 't'
  []
[]

[FunctorMaterials]
  [face_velocity]
    type = ADGenericVectorFunctorMaterial
    prop_names = face_velocity
    prop_values = 'vel_bar_x vel_bar_y 0'
  []
  [vel_inlet]
    type = GenericVectorFunctorMaterial
    prop_names = vel_inlet
    prop_values = 'u_inlet 0 0'
  []
  [vel_walls]
    type = GenericVectorFunctorMaterial
    prop_names = vel_walls
    prop_values = '0 0 0'
  []
[]

[Postprocessors]
  [reynolds]
    type = FunctionValuePostprocessor
    function = reynolds
  []
  [dofs]
    type = NumDOFs
  []
  [elems]
    type = NumElements
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options = '-nl0_condensed_ksp_view'
  petsc_options_iname = '-ksp_type'
  petsc_options_value = 'preonly'
  nl_abs_tol = 1e-7
  [TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '1 10 50 100 150 200 250 350 400 600 800 1000 1200 1400 1600 1800 2000 2200 2400 2600 2800 3000 3200 3400 3600 3800 4000 4200 4400 4600 4800 5000 5200 5400 5600 5800 6000 6200 6400 6600 6800 7000 7200 7400 7600 7800 8000 8200 8400 8600 8800 9000 9200 9400 9600 9800 10000'
    use_last_t_for_end_time = true
  []
  abort_on_solve_fail = true
[]

[Outputs]
  print_linear_residuals = false
  exodus = false
  checkpoint = true
  perf_graph = true
[]

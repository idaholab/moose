rho = 1
l = 1
U = 1
n = 4
gamma = 1e4
degree = 2
alpha = '${fparse 10 * degree^2}'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = ${l}
    zmin = 0
    zmax = ${l}
    nx = ${n}
    ny = ${n}
    nz = ${n}
    elem_type = TET10
  []
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
    family = LAGRANGE
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

[BCs]
  [momentum_x_diffusion_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left bottom right back front'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_x_diffusion_top]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'top'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '${U}'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_y_diffusion_all]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left bottom right top back front'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    component = 1
  []
  [momentum_z_diffusion_all]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left bottom right top back front'
    variable = vel_z
    face_variable = vel_bar_z
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    component = 2
  []

  [pressure_walls]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = 'left bottom right back front'
    face_velocity_functors = '0 0 0'
    interior_velocity_vars = 'vel_x vel_y vel_z'
  []
  [pressure_lid]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = 'top'
    face_velocity_functors = '${U} 0 0'
    interior_velocity_vars = 'vel_x vel_y vel_z'
  []

  [pb_mass]
    type = MassMatrixIntegratedBC
    variable = pressure_bar
    matrix_tags = 'mass'
    boundary = 'left right bottom top back front'
    density = '${fparse -1/gamma}'
  []

  [u_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    w = vel_z
    component = 0
    boundary = 'left right bottom back front'
    gamma = ${gamma}
    face_velocity = walls
    dirichlet_boundary = true
  []
  [v_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    w = vel_z
    component = 1
    boundary = 'left right bottom back front'
    gamma = ${gamma}
    face_velocity = walls
    dirichlet_boundary = true
  []
  [w_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_z
    face_variable = vel_bar_z
    u = vel_x
    v = vel_y
    w = vel_z
    component = 2
    boundary = 'left right bottom back front'
    gamma = ${gamma}
    face_velocity = walls
    dirichlet_boundary = true
  []
  [u_jump_top]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    w = vel_z
    component = 0
    boundary = 'top'
    gamma = ${gamma}
    face_velocity = top_vel
    dirichlet_boundary = true
  []
  [v_jump_top]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    w = vel_z
    component = 1
    boundary = 'top'
    gamma = ${gamma}
    face_velocity = top_vel
    dirichlet_boundary = true
  []
  [w_jump_top]
    type = MassFluxPenaltyBC
    variable = vel_z
    face_variable = vel_bar_z
    u = vel_x
    v = vel_y
    w = vel_z
    component = 2
    boundary = 'top'
    gamma = ${gamma}
    face_velocity = top_vel
    dirichlet_boundary = true
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
  [top]
    type = GenericVectorFunctorMaterial
    prop_names = top_vel
    prop_values = '${U} 0 0'
  []
  [walls]
    type = GenericVectorFunctorMaterial
    prop_names = walls
    prop_values = '0 0 0'
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
  print_linear_residuals = 'false'
  exodus = false
  checkpoint = true
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

[Correctors]
  [set_pressure]
    type = NSPressurePin
    pin_type = 'average'
    variable = pressure
    pressure_average = 'pressure_average'
  []
[]

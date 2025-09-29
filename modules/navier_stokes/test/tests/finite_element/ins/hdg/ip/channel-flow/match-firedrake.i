mu = 1e-2
rho = 1
n = 1
degree = 1
alpha = '${fparse 10*degree^2}'
gamma = 2

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = ${n}
    ny = ${n}
    elem_type = TRI6
  []
[]

[Variables]
  [vel_x]
    family = L2_LAGRANGE
    order = FIRST
  []
  [vel_y]
    family = L2_LAGRANGE
    order = FIRST
  []
  [pressure]
    family = MONOMIAL
    order = CONSTANT
  []
  [vel_bar_x]
    family = LAGRANGE
    order = FIRST
  []
  [vel_bar_y]
    family = LAGRANGE
    order = FIRST
  []
  [pressure_bar]
    family = LAGRANGE
    order = FIRST
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
    face_velocity = face_vel
    u = vel_x
    v = vel_y
    component = 0
    gamma = ${gamma}
  []
  [v_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_y
    face_variable = vel_bar_y
    face_velocity = face_vel
    u = vel_x
    v = vel_y
    component = 1
    gamma = ${gamma}
  []
[]

[BCs]
  #
  # dirichlet
  #
  [momentum_x_advection_dirichlet_inlet]
    type = AdvectionIPHDGDirichletBC
    boundary = 'left'
    face_variable = vel_bar_x
    functor = '1'
    variable = vel_x
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_advection_dirichlet_walls]
    type = AdvectionIPHDGDirichletBC
    boundary = 'top bottom'
    face_variable = vel_bar_x
    functor = '0'
    variable = vel_x
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_y_advection_dirichlet]
    type = AdvectionIPHDGDirichletBC
    boundary = 'top bottom left'
    face_variable = vel_bar_y
    functor = '0'
    variable = vel_y
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_diffusion_dirichlet_inlet]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '1'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_x_diffusion_dirichlet_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'top bottom'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_y_diffusion_dirichlet]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left top bottom'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    component = 1
  []
  [mass_dirichlet_inlet]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = 'left'
    face_velocity_functors = '1 0'
    interior_velocity_vars = 'vel_x vel_y'
  []
  [mass_dirichlet_walls]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = 'top bottom'
    face_velocity_functors = '0 0'
    interior_velocity_vars = 'vel_x vel_y'
  []

  #
  # Neumann
  #
  [momentum_x_advection_neumann]
    type = AdvectionIPHDGOutflowBC
    boundary = 'right'
    constrain_lm = false
    face_variable = vel_bar_x
    variable = vel_x
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_y_advection_neumann]
    type = AdvectionIPHDGOutflowBC
    boundary = 'right'
    constrain_lm = false
    face_variable = vel_bar_y
    variable = vel_y
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_diffusion_neumann]
    type = NavierStokesStressIPHDGPrescribedFluxBC
    boundary = 'right'
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
    boundary = 'right'
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
    boundary = 'right'
    face_velocity_functors = 'vel_bar_x vel_bar_y'
    interior_velocity_vars = 'vel_x vel_y'
  []

  [u_jump_inlet]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    component = 0
    gamma = ${gamma}
    face_velocity = inlet_face_vel
    boundary = 'left'
    dirichlet_boundary = true
  []
  [v_jump_inlet]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    component = 1
    gamma = ${gamma}
    face_velocity = inlet_face_vel
    boundary = 'left'
    dirichlet_boundary = true
  []
  [u_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    face_velocity = walls_face_vel
    u = vel_x
    v = vel_y
    component = 0
    gamma = ${gamma}
    boundary = 'top bottom'
    dirichlet_boundary = true
  []
  [v_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    component = 1
    gamma = ${gamma}
    face_velocity = walls_face_vel
    boundary = 'top bottom'
    dirichlet_boundary = true
  []
  [u_jump_outlet]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    component = 0
    gamma = ${gamma}
    face_velocity = face_vel
    boundary = 'right'
    dirichlet_boundary = false
  []
  [v_jump_outlet]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    component = 1
    gamma = ${gamma}
    face_velocity = face_vel
    boundary = 'right'
    dirichlet_boundary = false
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
  [vel]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = 'velocity'
    u = vel_x
    v = vel_y
  []
[]

[FunctorMaterials]
  [inlet]
    type = GenericConstantVectorFunctorMaterial
    prop_names = 'inlet_face_vel'
    prop_values = '1 0 0'
  []
  [walls]
    type = GenericConstantVectorFunctorMaterial
    prop_names = 'walls_face_vel'
    prop_values = '0 0 0'
  []
  [outlet]
    type = ADGenericConstantVectorFunctorMaterial
    prop_names = 'face_vel'
    prop_values = 'vel_bar_x vel_bar_y 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options = '-snes_monitor'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
  nl_rel_tol = 1e-12
  line_search = none
[]

[Outputs]
  csv = true
  exodus = true
  print_nonlinear_residuals = false
[]

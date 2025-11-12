!include ../3d-core.i

[Mesh]
  [file]
    type = FileMeshGenerator
    file = coarse13.msh
  []
  [conversion]
    type = ElementOrderConversionGenerator
    input = file
    conversion_type = COMPLETE_ORDER
  []
  # second_order = true
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
  [momentum_z_diffusion_inlet]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = '1'
    variable = vel_z
    face_variable = vel_bar_z
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = 0
    diffusivity = 'mu'
    component = 2
  []
  [mass_inlet]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = '1'
    face_velocity_functors = 'u_inlet 0 0'
    interior_velocity_vars = 'vel_x vel_y vel_z'
  []

  #
  # walls
  #
  [momentum_x_diffusion_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = '3'
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
    boundary = '3'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = 0
    diffusivity = 'mu'
    component = 1
  []
  [momentum_z_diffusion_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = '3'
    variable = vel_z
    face_variable = vel_bar_z
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = 0
    diffusivity = 'mu'
    component = 2
  []
  [mass_walls]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = '3'
    face_velocity_functors = '0 0 0'
    interior_velocity_vars = 'vel_x vel_y vel_z'
  []

  #
  # Neumann
  #
  [momentum_x_advection_neumann]
    type = AdvectionIPHDGOutflowBC
    boundary = '2'
    constrain_lm = false
    face_variable = vel_bar_x
    variable = vel_x
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_y_advection_neumann]
    type = AdvectionIPHDGOutflowBC
    boundary = '2'
    constrain_lm = false
    face_variable = vel_bar_y
    variable = vel_y
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_z_advection_neumann]
    type = AdvectionIPHDGOutflowBC
    boundary = '2'
    constrain_lm = false
    face_variable = vel_bar_z
    variable = vel_z
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_diffusion_neumann]
    type = NavierStokesStressIPHDGPrescribedFluxBC
    boundary = '2'
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
    boundary = '2'
    component = 1
    diffusivity = 'mu'
    face_variable = vel_bar_y
    prescribed_normal_flux = 0
    pressure_face_variable = pressure_bar
    pressure_variable = pressure
    variable = vel_y
    alpha = ${alpha}
  []
  [momentum_z_diffusion_neumann]
    type = NavierStokesStressIPHDGPrescribedFluxBC
    boundary = '2'
    component = 2
    diffusivity = 'mu'
    face_variable = vel_bar_z
    prescribed_normal_flux = 0
    pressure_face_variable = pressure_bar
    pressure_variable = pressure
    variable = vel_z
    alpha = ${alpha}
  []
  [mass_neumann]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = '2'
    face_velocity_functors = 'vel_bar_x vel_bar_y vel_bar_z'
    interior_velocity_vars = 'vel_x vel_y vel_z'
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
    w = vel_z
    component = 0
    boundary = '3'
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
    w = vel_z
    component = 1
    boundary = '3'
    gamma = ${gamma}
    face_velocity = vel_walls
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
    boundary = '3'
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
    w = vel_z
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
    w = vel_z
    component = 1
    boundary = '1'
    gamma = ${gamma}
    face_velocity = vel_inlet
    dirichlet_boundary = true
  []
  [w_jump_inlet]
    type = MassFluxPenaltyBC
    variable = vel_z
    face_variable = vel_bar_z
    u = vel_x
    v = vel_y
    w = vel_z
    component = 2
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
    w = vel_z
    component = 0
    boundary = '2'
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
    w = vel_z
    component = 1
    boundary = '2'
    gamma = ${gamma}
    face_velocity = face_velocity
    dirichlet_boundary = false
  []
  [w_jump_outlet]
    type = MassFluxPenaltyBC
    variable = vel_z
    face_variable = vel_bar_z
    u = vel_x
    v = vel_y
    w = vel_z
    component = 2
    boundary = '2'
    gamma = ${gamma}
    face_velocity = face_velocity
    dirichlet_boundary = false
  []
[]

[Functions]
  [u_inlet]
    type = ParsedFunction
    expression = '16*(2-y)*(y-1)*z*(1-z)'
  []
[]

[FunctorMaterials]
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

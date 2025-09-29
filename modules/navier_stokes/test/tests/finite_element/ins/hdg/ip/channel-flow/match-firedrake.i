mu = 1e-2
rho = 1
n = 1
degree = 1
alpha = '${fparse 10*degree^2}'

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
  # [pressure]
  #   family = MONOMIAL
  #   order = CONSTANT
  # []
  [vel_bar_x]
    family = LAGRANGE
    order = FIRST
  []
  [vel_bar_y]
    family = LAGRANGE
    order = FIRST
  []
  # [pressure_bar]
  #   family = LAGRANGE
  #   order = FIRST
  # []
[]

[HDGKernels]
  # [momentum_x_advection]
  #   type = AdvectionIPHDGKernel
  #   variable = vel_x
  #   face_variable = vel_bar_x
  #   velocity = 'velocity'
  #   coeff = ${rho}
  # []
  [momentum_x_diffusion]
    type = DiffusionIPHDGKernel
    variable = vel_x
    face_variable = vel_bar_x
    diffusivity = 'mu'
    alpha = ${alpha}
    # pressure_variable = pressure
    # pressure_face_variable = pressure_bar
    # component = 0
  []
  # [momentum_y_advection]
  #   type = AdvectionIPHDGKernel
  #   variable = vel_y
  #   face_variable = vel_bar_y
  #   velocity = 'velocity'
  #   coeff = ${rho}
  # []
  [momentum_y_diffusion]
    type = DiffusionIPHDGKernel
    variable = vel_y
    face_variable = vel_bar_y
    diffusivity = 'mu'
    alpha = ${alpha}
    # pressure_variable = pressure
    # pressure_face_variable = pressure_bar
    # component = 1
  []
  # [pressure]
  #   type = MassContinuityIPHDGKernel
  #   variable = pressure
  #   face_variable = pressure_bar
  #   interior_velocity_vars = 'vel_x vel_y'
  #   face_velocity_functors = 'vel_bar_x vel_bar_y'
  # []
[]

[BCs]
  #
  # dirichlet
  #
  # [momentum_x_advection_dirichlet]
  #   type = AdvectionIPHDGDirichletBC
  #   boundary = 'top bottom left'
  #   face_variable = vel_bar_x
  #   functor = exact_u
  #   variable = vel_x
  #   velocity = velocity
  #   coeff = ${rho}
  # []
  # [momentum_y_advection_dirichlet]
  #   type = AdvectionIPHDGDirichletBC
  #   boundary = 'top bottom left'
  #   face_variable = vel_bar_y
  #   functor = exact_v
  #   variable = vel_y
  #   velocity = velocity
  #   coeff = ${rho}
  # []
  [momentum_x_diffusion_dirichlet_inlet]
    type = DiffusionIPHDGDirichletBC
    boundary = 'left'
    variable = vel_x
    face_variable = vel_bar_x
    # pressure_variable = pressure
    # pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '1'
    diffusivity = 'mu'
    # component = 0
  []
  [momentum_x_diffusion_dirichlet_walls]
    type = DiffusionIPHDGDirichletBC
    boundary = 'top bottom'
    variable = vel_x
    face_variable = vel_bar_x
    # pressure_variable = pressure
    # pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    # component = 0
  []
  [momentum_y_diffusion_dirichlet]
    type = DiffusionIPHDGDirichletBC
    boundary = 'left top bottom'
    variable = vel_y
    face_variable = vel_bar_y
    # pressure_variable = pressure
    # pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    # component = 1
  []
  # [mass_dirichlet_inlet]
  #   type = MassContinuityIPHDGBC
  #   face_variable = pressure_bar
  #   variable = pressure
  #   boundary = 'left'
  #   face_velocity_functors = '1 0'
  #   interior_velocity_vars = 'vel_x vel_y'
  # []
  # [mass_dirichlet_walls]
  #   type = MassContinuityIPHDGBC
  #   face_variable = pressure_bar
  #   variable = pressure
  #   boundary = 'top bottom'
  #   face_velocity_functors = '0 0'
  #   interior_velocity_vars = 'vel_x vel_y'
  # []

  #
  # Neumann
  #
  # [momentum_x_advection_neumann]
  #   type = AdvectionIPHDGOutflowBC
  #   boundary = 'right'
  #   constrain_lm = false
  #   face_variable = vel_bar_x
  #   variable = vel_x
  #   velocity = velocity
  #   coeff = ${rho}
  # []
  # [momentum_y_advection_neumann]
  #   type = AdvectionIPHDGOutflowBC
  #   boundary = 'right'
  #   constrain_lm = false
  #   face_variable = vel_bar_y
  #   variable = vel_y
  #   velocity = velocity
  #   coeff = ${rho}
  # []
  [momentum_x_diffusion_neumann]
    type = DiffusionIPHDGPrescribedFluxBC
    boundary = 'right'
    # component = 0
    diffusivity = 'mu'
    face_variable = vel_bar_x
    prescribed_normal_flux = 0
    # pressure_face_variable = pressure_bar
    # pressure_variable = pressure
    variable = vel_x
    alpha = ${alpha}
  []
  [momentum_y_diffusion_neumann]
    type = DiffusionIPHDGPrescribedFluxBC
    boundary = 'right'
    # component = 1
    diffusivity = 'mu'
    face_variable = vel_bar_y
    prescribed_normal_flux = 0
    # pressure_face_variable = pressure_bar
    # pressure_variable = pressure
    variable = vel_y
    alpha = ${alpha}
  []
  # [mass_neumann]
  #   type = MassContinuityIPHDGBC
  #   face_variable = pressure_bar
  #   variable = pressure
  #   boundary = 'right'
  #   face_velocity_functors = 'vel_bar_x vel_bar_y'
  #   interior_velocity_vars = 'vel_x vel_y'
  # []
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

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
  nl_rel_tol = 1e-12
[]

[Outputs]
  csv = true
  exodus = true
[]

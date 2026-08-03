mu = 1
rho = 1

[GlobalParams]
  alpha = 6
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 10
    ny = 2
    elem_type = TRI6
  []
[]

[Variables]
  [vel_x]
    family = L2_HIERARCHIC
    order = FIRST
  []
  [vel_y]
    family = L2_HIERARCHIC
    order = FIRST
  []
  [pressure]
    family = MONOMIAL
    order = CONSTANT
  []
  [vel_bar_x]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
  [vel_bar_y]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
  [pressure_bar]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
  [temperature]
    family = L2_HIERARCHIC
    order = FIRST
  []
  [temperature_bar]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
[]

[HDGKernels]
  [momentum_x_advection]
    type = AdvectionIPHDGKernel
    variable = vel_x
    face_variable = vel_bar_x
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_diffusion]
    type = NavierStokesStressIPHDGKernel
    variable = vel_x
    face_variable = vel_bar_x
    diffusivity = mu
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    component = 0
  []
  [momentum_y_advection]
    type = AdvectionIPHDGKernel
    variable = vel_y
    face_variable = vel_bar_y
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_y_diffusion]
    type = NavierStokesStressIPHDGKernel
    variable = vel_y
    face_variable = vel_bar_y
    diffusivity = mu
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
  [temperature_advection]
    type = AdvectionIPHDGKernel
    variable = temperature
    face_variable = temperature_bar
    velocity = velocity
    coeff = 1
  []
  [temperature_diffusion]
    type = DiffusionIPHDGKernel
    variable = temperature
    face_variable = temperature_bar
    diffusivity = thermal_conductivity
  []
[]

[Functions]
  [u_inlet]
    type = ParsedFunction
    expression = '1.5*(1-y*y)'
  []
[]

[BCs]
  [momentum_x_advection_dirichlet]
    type = AdvectionIPHDGDirichletBC
    boundary = 'left top bottom'
    variable = vel_x
    face_variable = vel_bar_x
    functor = u_inlet
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_y_advection_dirichlet]
    type = AdvectionIPHDGDirichletBC
    boundary = 'left top bottom'
    variable = vel_y
    face_variable = vel_bar_y
    functor = 0
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_diffusion_dirichlet]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left top bottom'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    functor = u_inlet
    diffusivity = mu
    component = 0
  []
  [momentum_y_diffusion_dirichlet]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left top bottom'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    functor = 0
    diffusivity = mu
    component = 1
  []
  [mass_dirichlet]
    type = MassContinuityIPHDGBC
    boundary = 'left top bottom'
    variable = pressure
    face_variable = pressure_bar
    face_velocity_functors = 'u_inlet 0'
    interior_velocity_vars = 'vel_x vel_y'
  []
  [momentum_x_advection_outflow]
    type = AdvectionIPHDGOutflowBC
    boundary = right
    constrain_lm = false
    variable = vel_x
    face_variable = vel_bar_x
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_y_advection_outflow]
    type = AdvectionIPHDGOutflowBC
    boundary = right
    constrain_lm = false
    variable = vel_y
    face_variable = vel_bar_y
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_diffusion_outflow]
    type = NavierStokesStressIPHDGPrescribedTractionBC
    boundary = right
    component = 0
    diffusivity = mu
    variable = vel_x
    face_variable = vel_bar_x
    prescribed_normal_flux = 0
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
  []
  [momentum_y_diffusion_outflow]
    type = NavierStokesStressIPHDGPrescribedTractionBC
    boundary = right
    component = 1
    diffusivity = mu
    variable = vel_y
    face_variable = vel_bar_y
    prescribed_normal_flux = 0
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
  []
  [mass_outflow]
    type = MassContinuityIPHDGBC
    boundary = right
    variable = pressure
    face_variable = pressure_bar
    face_velocity_functors = 'vel_bar_x vel_bar_y'
    interior_velocity_vars = 'vel_x vel_y'
  []
  [temperature_advection_inlet]
    type = AdvectionIPHDGDirichletBC
    boundary = left
    variable = temperature
    face_variable = temperature_bar
    functor = 1
    velocity = velocity
    coeff = 1
  []
  [temperature_diffusion_inlet]
    type = DiffusionIPHDGDirichletBC
    boundary = left
    variable = temperature
    face_variable = temperature_bar
    functor = 1
    diffusivity = thermal_conductivity
  []
  [temperature_diffusion_insulated]
    type = DiffusionIPHDGPrescribedFluxBC
    boundary = 'top bottom right'
    variable = temperature
    face_variable = temperature_bar
    prescribed_normal_flux = 0
    diffusivity = thermal_conductivity
  []
  [temperature_advection_outflow]
    type = AdvectionIPHDGOutflowBC
    boundary = right
    constrain_lm = false
    variable = temperature
    face_variable = temperature_bar
    velocity = velocity
    coeff = 1
  []
[]

[Materials]
  [constants]
    type = ADGenericConstantMaterial
    prop_names = 'mu thermal_conductivity'
    prop_values = '${mu} 1'
  []
  [velocity]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = velocity
    u = vel_x
    v = vel_y
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  nl_rel_tol = 1e-12
[]

[Postprocessors]
  [outlet_velocity]
    type = SideAverageValue
    boundary = right
    variable = vel_x
  []
  [outlet_temperature]
    type = SideAverageValue
    boundary = right
    variable = temperature
  []
[]

[Outputs]
  csv = true
[]

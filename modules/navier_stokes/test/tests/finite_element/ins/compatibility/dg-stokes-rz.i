mu = 1
sigma = 20

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 1
    xmax = 2
    ymin = 0
    ymax = 1
    nx = 4
    ny = 4
    elem_type = TRI3
  []
  coord_type = RZ
  rz_coord_axis = Y
[]

[Variables]
  [vel_r]
    family = MONOMIAL
    order = FIRST
  []
  [vel_z]
    family = MONOMIAL
    order = FIRST
  []
  [pressure]
    family = MONOMIAL
    order = CONSTANT
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[Kernels]
  [momentum_r_diffusion]
    type = MatDiffusion
    variable = vel_r
    diffusivity = mu
  []
  [momentum_r_radial_viscous]
    type = ADMatReaction
    variable = vel_r
    reaction_rate = negative_mu_over_r_squared
  []
  [momentum_r_pressure]
    type = PressureGradient
    variable = vel_r
    pressure = pressure
    component = 0
    integrate_p_by_parts = true
  []
  [momentum_r_forcing]
    type = BodyForce
    variable = vel_r
    function = forcing_r
  []
  [momentum_z_diffusion]
    type = MatDiffusion
    variable = vel_z
    diffusivity = mu
  []
  [momentum_z_pressure]
    type = PressureGradient
    variable = vel_z
    pressure = pressure
    component = 1
    integrate_p_by_parts = true
  []
  [momentum_z_forcing]
    type = BodyForce
    variable = vel_z
    function = forcing_z
  []
  [mass]
    type = ADConservativeAdvection
    variable = pressure
    velocity_material = velocity
    advected_quantity = -1
  []
  [mean_zero_pressure]
    type = ScalarLagrangeMultiplier
    variable = pressure
    lambda = lambda
  []
[]

[ScalarKernels]
  [mean_zero_pressure_lm]
    type = AverageValueConstraint
    variable = lambda
    pp_name = pressure_integral
    value = 0
  []
[]

[DGKernels]
  [momentum_r_diffusion]
    type = DGDiffusion
    variable = vel_r
    sigma = ${sigma}
    epsilon = -1
    diff = mu
  []
  [momentum_z_diffusion]
    type = DGDiffusion
    variable = vel_z
    sigma = ${sigma}
    epsilon = -1
    diff = mu
  []
  [momentum_r_pressure]
    type = INSPressureGradientDGKernel
    variable = vel_r
    pressure = pressure
    component = 0
  []
  [momentum_z_pressure]
    type = INSPressureGradientDGKernel
    variable = vel_z
    pressure = pressure
    component = 1
  []
  [mass]
    type = ADDGAdvection
    variable = pressure
    velocity = velocity
    advected_quantity = -1
  []
[]

[BCs]
  [momentum_r_diffusion]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left right bottom top'
    variable = vel_r
    sigma = ${sigma}
    epsilon = -1
    function = exact_vel_r
    diff = mu
  []
  [momentum_z_diffusion]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left right bottom top'
    variable = vel_z
    sigma = ${sigma}
    epsilon = -1
    function = exact_vel_z
    diff = mu
  []
  [momentum_r_pressure]
    type = INSPressureGradientBC
    boundary = 'left right bottom top'
    variable = vel_r
    pressure = pressure
    component = 0
  []
  [momentum_z_pressure]
    type = INSPressureGradientBC
    boundary = 'left right bottom top'
    variable = vel_z
    pressure = pressure
    component = 1
  []
  [mass]
    type = ADConservativeAdvectionBC
    boundary = 'left right bottom top'
    variable = pressure
    velocity_function = exact_velocity
    advected_quantity = -1
  []
[]

[Functions]
  [exact_vel_r]
    type = ParsedFunction
    expression = '-pi*x*sin(pi*(x-1))*cos(pi*y)'
  []
  [exact_vel_z]
    type = ParsedFunction
    expression = '(2*sin(pi*(x-1))+pi*x*cos(pi*(x-1)))*sin(pi*y)'
  []
  [exact_pressure]
    type = ParsedFunction
    expression = 'sin(pi*(x-1))*cos(2*pi*y)'
  []
  [exact_velocity]
    type = ParsedVectorFunction
    expression_x = '-pi*x*sin(pi*(x-1))*cos(pi*y)'
    expression_y = '(2*sin(pi*(x-1))+pi*x*cos(pi*(x-1)))*sin(pi*y)'
  []
  [forcing_r]
    type = ParsedFunction
    symbol_names = mu
    symbol_values = ${mu}
    expression = 'mu*(-2*pi^3*x*sin(pi*(x-1))+3*pi^2*cos(pi*(x-1)))*cos(pi*y)+pi*cos(pi*(x-1))*cos(2*pi*y)'
  []
  [forcing_z]
    type = ParsedFunction
    symbol_names = mu
    symbol_values = ${mu}
    expression = 'pi*(mu*(2*pi^2*x*cos(pi*(x-1))+7*pi*sin(pi*(x-1))-3*cos(pi*(x-1))/x)-4*sin(pi*(x-1))*cos(pi*y))*sin(pi*y)'
  []
  [negative_mu_over_r_squared]
    type = ParsedFunction
    symbol_names = mu
    symbol_values = ${mu}
    expression = '-mu/(x*x)'
  []
[]

[Materials]
  [diffusivity]
    type = GenericConstantMaterial
    prop_names = mu
    prop_values = ${mu}
  []
  [velocity]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = velocity
    u = vel_r
    v = vel_z
  []
  [radial_reaction]
    type = ADGenericFunctionMaterial
    prop_names = negative_mu_over_r_squared
    prop_values = negative_mu_over_r_squared
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-12
[]

[Postprocessors]
  [pressure_integral]
    type = ElementIntegralVariablePostprocessor
    variable = pressure
    execute_on = linear
  []
  [h]
    type = AverageElementSize
    execute_on = timestep_end
  []
  [L2_vel_r]
    type = ElementL2Error
    variable = vel_r
    function = exact_vel_r
    execute_on = timestep_end
  []
  [L2_vel_z]
    type = ElementL2Error
    variable = vel_z
    function = exact_vel_z
    execute_on = timestep_end
  []
  [L2_pressure]
    type = ElementL2Error
    variable = pressure
    function = exact_pressure
    execute_on = timestep_end
  []
[]

[Outputs]
  csv = true
[]

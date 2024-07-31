mu = 2
rho = 2

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 2
    ny = 2
    elem_type = TRI6
  []
[]

[Variables]
  [vel_x]
    family = MONOMIAL
    order = FIRST
  []
  [vel_y]
    family = MONOMIAL
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
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[Kernels]
  [momentum_x_convection]
    type = ADConservativeAdvection
    variable = vel_x
    velocity = 'velocity'
    advected_quantity = 'rhou'
  []
  [momentum_x_diffusion]
    type = ADMatDiffusion
    variable = vel_x
    diffusivity = 'mu'
  []
  [momentum_x_pressure]
    type = PressureGradient
    integrate_p_by_parts = true
    variable = vel_x
    pressure = pressure
    component = 0
  []
  [momentum_x_ffn]
    type = BodyForce
    variable = vel_x
    function = forcing_u
  []

  [momentum_y_convection]
    type = ADConservativeAdvection
    variable = vel_y
    velocity = 'velocity'
    advected_quantity = 'rhov'
  []
  [momentum_y_diffusion]
    type = ADMatDiffusion
    variable = vel_y
    diffusivity = 'mu'
  []
  [momentum_y_pressure]
    type = PressureGradient
    integrate_p_by_parts = true
    variable = vel_y
    pressure = pressure
    component = 1
  []
  [momentum_y_ffn]
    type = BodyForce
    variable = vel_y
    function = forcing_v
  []

  [mass]
    type = ADConservativeAdvection
    variable = pressure
    velocity = velocity
    advected_quantity = ${fparse -rho}
  []
  [mass_ffn]
    type = BodyForce
    variable = pressure
    function = forcing_p
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
  [momentum_x_convection]
    type = ADHDGAdvection
    variable = vel_x
    velocity = 'velocity'
    coeff = ${rho}
    side_variable = vel_bar_x
  []
  [momentum_x_convection_side]
    type = ADHDGAdvectionSide
    variable = vel_bar_x
    velocity = 'velocity'
    coeff = ${rho}
    interior_variable = vel_x
  []
  [momentum_x_diffusion]
    type = ADHDGDiffusion
    variable = vel_x
    alpha = 6
    diff = 'mu'
    side_variable = vel_bar_x
  []
  [momentum_x_diffusion_side]
    type = ADHDGDiffusionSide
    variable = vel_bar_x
    alpha = 6
    diff = 'mu'
    interior_variable = vel_x
  []
  [momentum_x_pressure]
    type = ADHDGPressure
    variable = vel_x
    pressure = pressure_bar
    component = 0
  []

  [momentum_y_convection]
    type = ADHDGAdvection
    variable = vel_y
    velocity = 'velocity'
    coeff = ${rho}
    side_variable = vel_bar_y
  []
  [momentum_y_convection_side]
    type = ADHDGAdvectionSide
    variable = vel_bar_y
    velocity = 'velocity'
    coeff = ${rho}
    interior_variable = vel_y
  []
  [momentum_y_diffusion]
    type = ADHDGDiffusion
    variable = vel_y
    alpha = 6
    diff = 'mu'
    side_variable = vel_bar_y
  []
  [momentum_y_diffusion_side]
    type = ADHDGDiffusionSide
    variable = vel_bar_y
    alpha = 6
    diff = 'mu'
    interior_variable = vel_y
  []
  [momentum_y_pressure]
    type = ADHDGPressure
    variable = vel_y
    pressure = pressure_bar
    component = 1
  []

  [mass_convection]
    type = ADHDGAdvection
    variable = pressure
    velocity = 'velocity'
    coeff = ${fparse -rho}
    self_advection = false
  []
  [mass_convection_bar]
    type = ADHDGAdvection
    variable = pressure_bar
    velocity = 'velocity'
    coeff = ${rho}
    self_advection = false
  []
[]

[BCs]
  [momentum_x_diffusion_all]
    type = HDGDiffusionBC
    boundary = 'left bottom right top'
    variable = vel_x
    alpha = 6
    exact_soln = exact_u
    diff = 'mu'
  []
  [momentum_x_diffusion_side_all]
    type = ADHDGSideDirichletBC
    variable = vel_bar_x
    exact_soln = exact_u
    boundary = 'left bottom right top'
  []
  [momentum_x_pressure_all]
    type = ADHDGPressureBC
    variable = vel_x
    component = 0
    pressure = pressure_bar
    boundary = 'left bottom right top'
  []

  [momentum_y_diffusion_all]
    type = HDGDiffusionBC
    boundary = 'left bottom right top'
    variable = vel_y
    alpha = 6
    exact_soln = exact_v
    diff = 'mu'
  []
  [momentum_y_side_all]
    type = ADHDGSideDirichletBC
    variable = vel_bar_y
    exact_soln = exact_v
    boundary = 'left bottom right top'
  []
  [momentum_y_pressure_all]
    type = ADHDGPressureBC
    variable = vel_y
    component = 1
    pressure = pressure_bar
    boundary = 'left bottom right top'
  []

  [mass_convection_all]
    type = ADHDGAdvectionDirichletBC
    variable = pressure
    velocity = 'velocity'
    coeff = ${fparse -rho}
    self_advection = false
    boundary = 'left bottom top right'
  []
  [mass_convection_bar_all]
    type = ADHDGAdvectionDirichletBC
    variable = pressure_bar
    velocity = 'velocity'
    coeff = ${rho}
    self_advection = false
    boundary = 'left bottom top right'
  []
  [mass_convection_bar_diri_all]
    type = ADHDGAdvectionDirichletBC
    variable = pressure_bar
    velocity_function = vector_vel_func
    coeff = ${fparse -rho}
    self_advection = false
    boundary = 'left bottom right top'
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'sin(y)*cos((1/2)*x*pi)'
  []
  [forcing_u]
    type = ParsedFunction
    expression = 'mu*sin(y)*cos((1/2)*x*pi) + (1/4)*pi^2*mu*sin(y)*cos((1/2)*x*pi) - 1/2*pi*rho*sin(x)*sin(y)*sin((1/2)*y*pi)*cos((1/2)*x*pi) + rho*sin(x)*cos(y)*cos((1/2)*x*pi)*cos((1/2)*y*pi) - pi*rho*sin(y)^2*sin((1/2)*x*pi)*cos((1/2)*x*pi) + sin(y)*cos(x)'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [exact_v]
    type = ParsedFunction
    expression = 'sin(x)*cos((1/2)*y*pi)'
  []
  [forcing_v]
    type = ParsedFunction
    expression = 'mu*sin(x)*cos((1/2)*y*pi) + (1/4)*pi^2*mu*sin(x)*cos((1/2)*y*pi) - pi*rho*sin(x)^2*sin((1/2)*y*pi)*cos((1/2)*y*pi) - 1/2*pi*rho*sin(x)*sin(y)*sin((1/2)*x*pi)*cos((1/2)*y*pi) + rho*sin(y)*cos(x)*cos((1/2)*x*pi)*cos((1/2)*y*pi) + sin(x)*cos(y)'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'sin(x)*sin(y)'
  []
  [forcing_p]
    type = ParsedFunction
    expression = '(1/2)*pi*rho*sin(x)*sin((1/2)*y*pi) + (1/2)*pi*rho*sin(y)*sin((1/2)*x*pi)'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
  [vector_vel_func]
    type = ParsedVectorFunction
    expression_x = 'sin(y)*cos((1/2)*x*pi)'
    expression_y = 'sin(x)*cos((1/2)*y*pi)'
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
  [rhou]
    type = ADParsedMaterial
    property_name = 'rhou'
    coupled_variables = 'vel_x'
    material_property_names = 'rho'
    expression = 'rho*vel_x'
  []
  [rhov]
    type = ADParsedMaterial
    property_name = 'rhov'
    coupled_variables = 'vel_y'
    material_property_names = 'rho'
    expression = 'rho*vel_y'
  []
[]

[AuxVariables]
  [vel_exact_x][]
  [vel_exact_y][]
  [pressure_exact][]
[]

[AuxKernels]
  [vel_exact_x]
    type = FunctionAux
    variable = vel_exact_x
    function = exact_u
  []
  [vel_exact_y]
    type = FunctionAux
    variable = vel_exact_y
    function = exact_v
  []
  [pressure_exact]
    type = FunctionAux
    variable = pressure_exact
    function = exact_p
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'lambda pressure_integral'
  []
  csv = true
[]

[Postprocessors]
  [pressure_integral]
    type = ElementIntegralVariablePostprocessor
    variable = pressure
    execute_on = linear
  []
  [h]
    type = AverageElementSize
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2Error
    variable = vel_x
    function = exact_u
    execute_on = 'timestep_end'
  []
  [L2v]
    type = ElementL2Error
    variable = vel_y
    function = exact_v
    execute_on = 'timestep_end'
  []
  [L2p]
    type = ElementL2Error
    variable = pressure
    function = exact_p
    execute_on = 'timestep_end'
  []
[]

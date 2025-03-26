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
    order = SECOND
  []
  [vel_y]
    family = MONOMIAL
    order = SECOND
  []
  [pressure]
    family = MONOMIAL
    order = FIRST
  []
  [vel_bar_x]
    family = SIDE_HIERARCHIC
    order = SECOND
  []
  [vel_bar_y]
    family = SIDE_HIERARCHIC
    order = SECOND
  []
  [pressure_bar]
    family = SIDE_HIERARCHIC
    order = SECOND
  []
  [lambda]
    family = SCALAR
    order = FIRST
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
    alpha = 6
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
    alpha = 6
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    component = 1
  []
  [pressure_convection]
    type = AdvectionIPHDGKernel
    variable = pressure
    face_variable = pressure_bar
    velocity = 'velocity'
    coeff = '${fparse -rho}'
    self_advection = false
  []
[]

[Kernels]
  [momentum_x_ffn]
    type = BodyForce
    variable = vel_x
    function = forcing_u
  []
  [momentum_y_ffn]
    type = BodyForce
    variable = vel_y
    function = forcing_v
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

[BCs]
  [momentum_x_diffusion_all]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left bottom right top'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = 6
    functor = exact_u
    diffusivity = 'mu'
    component = 0
  []
  [momentum_y_diffusion_all]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left bottom right top'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = 6
    functor = exact_v
    diffusivity = 'mu'
    component = 1
  []

  [mass_convection]
    type = AdvectionIPHDGPrescribedFluxBC
    face_variable = pressure_bar
    variable = pressure
    velocity = 'velocity'
    coeff = '${fparse -rho}'
    self_advection = false
    boundary = 'left bottom top right'
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

[AuxVariables]
  [vel_exact_x]
  []
  [vel_exact_y]
  []
  [pressure_exact]
  []
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

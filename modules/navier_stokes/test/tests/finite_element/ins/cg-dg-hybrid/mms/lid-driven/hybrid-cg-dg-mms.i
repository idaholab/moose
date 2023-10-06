rho=1.1
mu=1.1
cp=1.1
k=1.1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1
    xmax = 1.0
    ymin = -1
    ymax = 1.0
    nx = 2
    ny = 2
  []
  [corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = gen
  []
[]

[Variables]
  [u]
    family = MONOMIAL
  []
  [v]
    family = MONOMIAL
  []
  [pressure][]
  [T]
    family = MONOMIAL
  []
[]

[Kernels]
  [momentum_x_convection]
    type = ADConservativeAdvection
    variable = u
    velocity = 'velocity'
    advected_quantity = 'rhou'
  []
  [momentum_x_diffusion]
    type = MatDiffusion
    variable = u
    diffusivity = 'mu'
  []
  [momentum_x_pressure]
    type = PressureGradient
    integrate_p_by_parts = false
    variable = u
    pressure = pressure
    component = 0
  []
  [u_forcing]
    type = BodyForce
    variable = u
    function = forcing_u
  []
  [momentum_y_convection]
    type = ADConservativeAdvection
    variable = v
    velocity = 'velocity'
    advected_quantity = 'rhov'
  []
  [momentum_y_diffusion]
    type = MatDiffusion
    variable = v
    diffusivity = 'mu'
  []
  [momentum_y_pressure]
    type = PressureGradient
    integrate_p_by_parts = false
    variable = v
    pressure = pressure
    component = 1
  []
  [v_forcing]
    type = BodyForce
    variable = v
    function = forcing_v
  []
  [mass]
    type = ADConservativeAdvection
    variable = pressure
    velocity = velocity
    advected_quantity = -1
  []
  [p_forcing]
    type = BodyForce
    variable = pressure
    function = forcing_p
  []
  [T_convection]
    type = ADConservativeAdvection
    variable = T
    velocity = 'velocity'
    advected_quantity = 'rho_cp_temp'
  []
  [T_diffusion]
    type = MatDiffusion
    variable = T
    diffusivity = 'k'
  []
  [T_forcing]
    type = BodyForce
    variable = T
    function = forcing_T
  []
[]

[DGKernels]
  [momentum_x_convection]
    type = ADDGAdvection
    variable = u
    velocity = 'velocity'
    advected_quantity = 'rhou'
  []
  [momentum_x_diffusion]
    type = DGDiffusion
    variable = u
    sigma = 6
    epsilon = -1
    diff = 'mu'
  []
  [momentum_y_convection]
    type = ADDGAdvection
    variable = v
    velocity = 'velocity'
    advected_quantity = 'rhov'
  []
  [momentum_y_diffusion]
    type = DGDiffusion
    variable = v
    sigma = 6
    epsilon = -1
    diff = 'mu'
  []
  [T_convection]
    type = ADDGAdvection
    variable = T
    velocity = 'velocity'
    advected_quantity = 'rho_cp_temp'
  []
  [T_diffusion]
    type = DGDiffusion
    variable = T
    sigma = 6
    epsilon = -1
    diff = 'k'
  []
[]

[BCs]
  [u_walls]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left bottom right top'
    variable = u
    sigma = 6
    epsilon = -1
    function = exact_u
    diff = 'mu'
  []
  [v_walls]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left bottom right top'
    variable = v
    sigma = 6
    epsilon = -1
    function = exact_v
    diff = 'mu'
  []
  [pressure_pin]
    type = FunctionDirichletBC
    variable = pressure
    boundary = 'pinned_node'
    function = 'exact_p'
  []
  [T_walls]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left bottom right top'
    variable = T
    sigma = 6
    epsilon = -1
    function = exact_T
    diff = 'k'
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho cp'
    prop_values = '${rho} ${cp}'
  []
  [const_reg]
    type = GenericConstantMaterial
    prop_names = 'mu k'
    prop_values = '${mu} ${k}'
  []
  [vel]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = 'velocity'
    u = u
    v = v
  []
  [rhou]
    type = ADParsedMaterial
    property_name = 'rhou'
    coupled_variables = 'u'
    material_property_names = 'rho'
    expression = 'rho*u'
  []
  [rhov]
    type = ADParsedMaterial
    property_name = 'rhov'
    coupled_variables = 'v'
    material_property_names = 'rho'
    expression = 'rho*v'
  []
  [rho_cp]
    type = ADParsedMaterial
    property_name = 'rho_cp'
    material_property_names = 'rho cp'
    expression = 'rho*cp'
  []
  [rho_cp_temp]
    type = ADParsedMaterial
    property_name = 'rho_cp_temp'
    material_property_names = 'rho cp'
    coupled_variables = 'T'
    expression = 'rho*cp*T'
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
    expression = '(1/2)*pi*sin(x)*sin((1/2)*y*pi) + (1/2)*pi*sin(y)*sin((1/2)*x*pi)'
  []
  [exact_T]
    type = ParsedFunction
    expression = 'cos(x)*cos(y)'
  []
  [forcing_T]
    type = ParsedFunction
    expression = '-cp*rho*sin(x)*sin(y)*cos(x)*cos((1/2)*y*pi) - cp*rho*sin(x)*sin(y)*cos(y)*cos((1/2)*x*pi) - 1/2*pi*cp*rho*sin(x)*sin((1/2)*y*pi)*cos(x)*cos(y) - 1/2*pi*cp*rho*sin(y)*sin((1/2)*x*pi)*cos(x)*cos(y) + 2*k*cos(x)*cos(y)'
    symbol_names = 'rho cp k'
    symbol_values = '${rho} ${cp} ${k}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2Error
    variable = u
    function = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2v]
    variable = v
    function = exact_v
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2T]
    variable = T
    function = exact_T
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2p]
    variable = pressure
    function = exact_p
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]

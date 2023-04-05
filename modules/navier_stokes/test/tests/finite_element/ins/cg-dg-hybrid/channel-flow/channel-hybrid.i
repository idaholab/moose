mu = 1.1
rho = 1.1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 100
    ny = 20
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
  [mass]
    type = ADConservativeAdvection
    variable = pressure
    velocity = velocity
    advected_quantity = -1
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
[]

[Functions]
  [v_inlet]
    type = ParsedVectorFunction
    expression_x = '1'
  []
[]

[BCs]
  [u_walls]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'bottom top'
    variable = u
    sigma = 6
    epsilon = -1
    function = '0'
    diff = 'mu'
  []
  [v_walls]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'bottom top'
    variable = v
    sigma = 6
    epsilon = -1
    function = '0'
    diff = 'mu'
  []
  [u_in]
    type = ADConservativeAdvectionBC
    boundary = 'left'
    variable = u
    velocity_function = v_inlet
    primal_dirichlet_value = 1
    primal_coefficient = 'rho'
  []
  [v_in]
    type = ADConservativeAdvectionBC
    boundary = 'left'
    variable = v
    velocity_function = v_inlet
    primal_dirichlet_value = 0
    primal_coefficient = 'rho'
  []
  [p_in]
    type = ADConservativeAdvectionBC
    boundary = 'left'
    variable = pressure
    velocity_function = v_inlet
    advected_quantity = -1
  []
  [u_out]
    type = ADConservativeAdvectionBC
    boundary = 'right'
    variable = u
    velocity_mat_prop = 'velocity'
    advected_quantity = 'rhou'
  []
  [v_out]
    type = ADConservativeAdvectionBC
    boundary = 'right'
    variable = v
    velocity_mat_prop = 'velocity'
    advected_quantity = 'rhov'
  []
  [p_out]
    type = DirichletBC
    variable = pressure
    boundary = 'right'
    value = 0
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho'
    prop_values = '${rho}'
  []
  [const_reg]
    type = GenericConstantMaterial
    prop_names = 'mu'
    prop_values = '${mu}'
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
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]

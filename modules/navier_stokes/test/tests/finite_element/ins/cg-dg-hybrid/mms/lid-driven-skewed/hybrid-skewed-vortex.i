rho=1
mu=1

[Mesh]
  [gen_mesh]
    type = FileMeshGenerator
    file = skewed.msh
  []
  [corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = gen_mesh
  []
[]

[Variables]
  [u]
    family = MONOMIAL
    order = SECOND
  []
  [v]
    family = MONOMIAL
    order = SECOND
  []
  [pressure][]
[]

[Kernels]
  [momentum_x_convection]
    type = ADConservativeAdvection
    variable = u
    velocity = 'velocity'
  []
  [momentum_x_diffusion]
    type = Diffusion
    variable = u
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
  []
  [momentum_y_diffusion]
    type = Diffusion
    variable = v
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
[]

[DGKernels]
  [momentum_x_convection]
    type = ADDGAdvection
    variable = u
    velocity = 'velocity'
  []
  [momentum_x_diffusion]
    type = DGDiffusion
    variable = u
    sigma = 6
    epsilon = -1
  []
  [momentum_y_convection]
    type = ADDGAdvection
    variable = v
    velocity = 'velocity'
  []
  [momentum_y_diffusion]
    type = DGDiffusion
    variable = v
    sigma = 6
    epsilon = -1
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
  []
  [v_walls]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left bottom right top'
    variable = v
    sigma = 6
    epsilon = -1
    function = exact_v
  []
  [pressure_pin]
    type = FunctionDirichletBC
    variable = pressure
    boundary = 'pinned_node'
    function = 'exact_p'
  []
[]

[Materials]
  [rho]
    type = ADGenericConstantMaterial
    prop_names = 'rho'
    prop_values = '${rho}'
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

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'x^2*(1-x)^2*(2*y-6*y^2+4*y^3)'
  []
  [exact_v]
    type = ParsedFunction
    expression = '-y^2*(1-y)^2*(2*x-6*x^2+4*x^3)'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'x*(1-x)-2/12'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '-4*mu/rho*(-1+2*y)*(y^2-6*x*y^2+6*x^2*y^2-y+6*x*y-6*x^2*y+3*x^2-6*x^3+3*x^4)+1-2*x+4*x^3'
            '*y^2*(2*y^2-2*y+1)*(y-1)^2*(-1+2*x)*(x-1)^3'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '4*mu/rho*(-1+2*x)*(x^2-6*y*x^2+6*x^2*y^2-x+6*x*y-6*x*y^2+3*y^2-6*y^3+3*y^4)+4*y^3*x^2*(2'
            '*x^2-2*x+1)*(x-1)^2*(-1+2*y)*(y-1)^3'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
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
  [L2p]
    variable = pressure
    function = exact_p
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]

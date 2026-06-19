mu = 1.1
rho = 1.1
vin = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 50
    ny = 10
    elem_type = tri3
  []
[]

[Variables]
  [u]
    family = MONOMIAL
  []
  [v]
    family = MONOMIAL
  []
  [pressure]
  []
[]

[Kernels]
  [momentum_x_diffusion]
    type = MatDiffusion
    variable = u
    diffusivity = 'mu'
  []
  [momentum_x_pressure]
    type = PressureGradient
    integrate_p_by_parts = true
    variable = u
    pressure = pressure
    component = 0
  []
  [momentum_y_diffusion]
    type = MatDiffusion
    variable = v
    diffusivity = 'mu'
  []
  [momentum_y_pressure]
    type = PressureGradient
    integrate_p_by_parts = true
    variable = v
    pressure = pressure
    component = 1
  []
  [mass]
    type = ADConservativeAdvection
    variable = pressure
    velocity_material = velocity
    advected_quantity = -1
  []
[]

[DGKernels]
  [momentum_x_diffusion]
    type = DGDiffusion
    variable = u
    sigma = 6
    epsilon = -1
    diff = 'mu'
  []
  [momentum_y_diffusion]
    type = DGDiffusion
    variable = v
    sigma = 6
    epsilon = -1
    diff = 'mu'
  []
  [momentum_x_pressure]
    type = INSPressureGradientDGKernel
    variable = u
    pressure = pressure
    component = 0
  []
  [momentum_y_pressure]
    type = INSPressureGradientDGKernel
    variable = v
    pressure = pressure
    component = 1
  []
[]

[Functions]
  [v_inlet]
    type = ParsedVectorFunction
    expression_x = '${vin}'
  []
[]

[BCs]
  [diffusion_momentum_x_in]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left'
    variable = u
    sigma = 6
    epsilon = -1
    function = '${vin}'
    diff = 'mu'
  []
  [diffusion_momentum_y_in]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left'
    variable = v
    sigma = 6
    epsilon = -1
    function = '0'
    diff = 'mu'
  []
  [diffusion_momentum_x_walls]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'bottom top'
    variable = u
    sigma = 6
    epsilon = -1
    function = '0'
    diff = 'mu'
  []
  [diffusion_momentum_y_walls]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'bottom top'
    variable = v
    sigma = 6
    epsilon = -1
    function = '0'
    diff = 'mu'
  []
  [advection_mass_in]
    type = ADConservativeAdvectionBC
    boundary = 'left'
    variable = pressure
    velocity_function = v_inlet
    advected_quantity = -1
  []
  [advection_mass_out]
    type = ADConservativeAdvectionBC
    boundary = 'right'
    variable = pressure
    velocity_mat_prop = 'velocity'
    advected_quantity = -1
  []
  [implicit_pressure_x_in_and_walls]
    type = INSPressureGradientBC
    boundary = 'left top bottom'
    component = 0
    pressure = pressure
    variable = u
  []
  [implicit_pressure_y_in_and_walls]
    type = INSPressureGradientBC
    boundary = 'left top bottom'
    component = 1
    pressure = pressure
    variable = v
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
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  nl_rel_tol = 1e-12
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  inactive = 'symmetry' # expensive except for low dof count
  [symmetry]
    type = MatrixSymmetryCheck
  []
  [side_average]
    type = SideAverageValue
    boundary = 'right'
    variable = u
  []
[]

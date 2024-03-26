mu = 1
rho = 1
l = 1
U = 1
n = 2

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = ${l}
    nx = ${n}
    ny = ${n}
    elem_type = QUAD9
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
  [pressure]
    family = MONOMIAL
    order = FIRST
  []
  [u_bar]
    family = SIDE_HIERARCHIC
    order = SECOND
  []
  [v_bar]
    family = SIDE_HIERARCHIC
    order = SECOND
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
  # [momentum_x_convection]
  #   type = ADConservativeAdvection
  #   variable = u
  #   velocity = 'velocity'
  #   advected_quantity = 'rhou'
  # []
  [momentum_x_diffusion]
    type = ADMatDiffusion
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

  # [momentum_y_convection]
  #   type = ADConservativeAdvection
  #   variable = v
  #   velocity = 'velocity'
  #   advected_quantity = 'rhov'
  # []
  [momentum_y_diffusion]
    type = ADMatDiffusion
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
    velocity = velocity
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
  # [momentum_x_convection]
  #   type = ADHDGAdvection
  #   variable = u
  #   velocity = 'velocity'
  #   coeff = ${rho}
  #   side_variable = u_bar
  # []
  # [momentum_x_convection_side]
  #   type = ADHDGAdvectionSide
  #   variable = u_bar
  #   velocity = 'velocity'
  #   coeff = ${rho}
  #   interior_variable = u
  # []
  [momentum_x_diffusion]
    type = ADHDGDiffusion
    variable = u
    alpha = 6
    diff = 'mu'
    side_variable = u_bar
  []
  [momentum_x_diffusion_side]
    type = ADHDGDiffusionSide
    variable = u_bar
    alpha = 6
    diff = 'mu'
    interior_variable = u
  []
  [momentum_x_pressure]
    type = ADHDGPressure
    variable = u
    pressure = pressure_bar
    component = 0
  []

  # [momentum_y_convection]
  #   type = ADHDGAdvection
  #   variable = v
  #   velocity = 'velocity'
  #   coeff = ${rho}
  #   side_variable = v_bar
  # []
  # [momentum_y_convection_side]
  #   type = ADHDGAdvectionSide
  #   variable = v_bar
  #   velocity = 'velocity'
  #   coeff = ${rho}
  #   interior_variable = v
  # []
  [momentum_y_diffusion]
    type = ADHDGDiffusion
    variable = v
    alpha = 6
    diff = 'mu'
    side_variable = v_bar
  []
  [momentum_y_diffusion_side]
    type = ADHDGDiffusionSide
    variable = v_bar
    alpha = 6
    diff = 'mu'
    interior_variable = v
  []
  [momentum_y_pressure]
    type = ADHDGPressure
    variable = v
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
  [momentum_x_diffusion_walls]
    type = HDGDiffusionBC
    boundary = 'left bottom right'
    variable = u
    alpha = 6
    exact_soln = '0'
    diff = 'mu'
  []
  [momentum_x_diffusion_side_walls]
    type = ADHDGSideDirichletBC
    variable = u_bar
    exact_soln = 0
    boundary = 'left bottom right'
  []
  [momentum_x_diffusion_top]
    type = HDGDiffusionBC
    boundary = 'top'
    variable = u
    alpha = 6
    exact_soln = '${U}'
    diff = 'mu'
  []
  [momentum_x_diffusion_side_top]
    type = ADHDGSideDirichletBC
    variable = u_bar
    exact_soln = ${U}
    boundary = 'top'
  []
  [momentum_x_pressure_all]
    type = ADHDGPressureBC
    variable = u
    component = 0
    pressure = pressure_bar
    boundary = 'left bottom right top'
  []

  [momentum_y_diffusion_all]
    type = HDGDiffusionBC
    boundary = 'left bottom right top'
    variable = v
    alpha = 6
    exact_soln = '0'
    diff = 'mu'
  []
  [momentum_y_side_all]
    type = ADHDGSideDirichletBC
    variable = v_bar
    exact_soln = 0
    boundary = 'left bottom right top'
  []
  [momentum_y_pressure_all]
    type = ADHDGPressureBC
    variable = v
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
  [mass_convection_bar_diri_walls]
    type = ADHDGAdvectionDirichletBC
    variable = pressure_bar
    velocity_function = wall_vel_func
    coeff = ${fparse -rho}
    self_advection = false
    boundary = 'left bottom right'
  []
  [mass_convection_bar_diri_top]
    type = ADHDGAdvectionDirichletBC
    variable = pressure_bar
    velocity_function = top_vel_func
    coeff = ${fparse -rho}
    self_advection = false
    boundary = 'top'
  []
[]

[Functions]
  [top_vel_func]
    type = ParsedVectorFunction
    expression_x = ${U}
  []
  [wall_vel_func]
    type = ParsedVectorFunction
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

[AuxVariables]
  [vel_x]
    family = MONOMIAL
    order = CONSTANT
  []
  [vel_y]
    family = MONOMIAL
    order = CONSTANT
  []
  [p]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [vel_x]
    type = ProjectionAux
    variable = vel_x
    v = u
  []
  [vel_y]
    type = ProjectionAux
    variable = vel_y
    v = v
  []
  [p]
    type = ProjectionAux
    variable = p
    v = pressure
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

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    pp_names = ''
    function = '${rho} * ${U} * ${l} / ${mu}'
  []
  [symmetric]
    type = IsMatrixSymmetric
  []
  [pressure_integral]
    type = ElementIntegralVariablePostprocessor
    variable = pressure
    execute_on = linear
  []
[]

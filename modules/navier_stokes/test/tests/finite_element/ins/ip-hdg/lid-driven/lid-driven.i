mu = 1
rho = 1
l = 1
U = 100
n = 8

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
  [vel_x]
    family = L2_HIERARCHIC
    order = SECOND
  []
  [vel_y]
    family = L2_HIERARCHIC
    order = SECOND
  []
  [pressure]
    family = L2_HIERARCHIC
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

  [mass]
    type = ADConservativeAdvection
    variable = pressure
    velocity = velocity
    advected_quantity = ${fparse -rho}
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
  [momentum_x_diffusion_walls]
    type = HDGDiffusionBC
    boundary = 'left bottom right'
    variable = vel_x
    alpha = 6
    exact_soln = '0'
    diff = 'mu'
  []
  [momentum_x_diffusion_side_walls]
    type = ADHDGSideDirichletBC
    variable = vel_bar_x
    exact_soln = 0
    boundary = 'left bottom right'
  []
  [momentum_x_diffusion_top]
    type = HDGDiffusionBC
    boundary = 'top'
    variable = vel_x
    alpha = 6
    exact_soln = '${U}'
    diff = 'mu'
  []
  [momentum_x_diffusion_side_top]
    type = ADHDGSideDirichletBC
    variable = vel_bar_x
    exact_soln = ${U}
    boundary = 'top'
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
    exact_soln = '0'
    diff = 'mu'
  []
  [momentum_y_side_all]
    type = ADHDGSideDirichletBC
    variable = vel_bar_y
    exact_soln = 0
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

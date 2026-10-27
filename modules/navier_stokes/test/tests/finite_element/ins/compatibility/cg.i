mu = 1
rho = 1
penalty = 1e6

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
  [u]
    family = LAGRANGE
    order = SECOND
  []
  [v]
    family = LAGRANGE
    order = SECOND
  []
  [pressure]
    family = LAGRANGE
    order = FIRST
  []
  [temperature]
    family = LAGRANGE
    order = FIRST
  []
[]

[Kernels]
  [momentum_x_convection]
    type = ADConservativeAdvection
    variable = u
    velocity_material = velocity
    advected_quantity = rhou
  []
  [momentum_x_diffusion]
    type = MatDiffusion
    variable = u
    diffusivity = mu
  []
  [momentum_x_pressure]
    type = PressureGradient
    variable = u
    pressure = pressure
    component = 0
  []
  [momentum_y_convection]
    type = ADConservativeAdvection
    variable = v
    velocity_material = velocity
    advected_quantity = rhov
  []
  [momentum_y_diffusion]
    type = MatDiffusion
    variable = v
    diffusivity = mu
  []
  [momentum_y_pressure]
    type = PressureGradient
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
  [temperature_convection]
    type = ADConservativeAdvection
    variable = temperature
    velocity_material = velocity
  []
  [temperature_diffusion]
    type = MatDiffusion
    variable = temperature
    diffusivity = 1
  []
[]

[Functions]
  [u_inlet]
    type = ParsedFunction
    expression = '1.5*(1-y*y)'
  []
  [v_inlet]
    type = ParsedVectorFunction
    expression_x = '1.5*(1-y*y)'
  []
[]

[BCs]
  [u_in]
    type = ADFunctionPenaltyDirichletBC
    boundary = left
    variable = u
    function = u_inlet
    penalty = ${penalty}
  []
  [v_in]
    type = ADPenaltyDirichletBC
    boundary = left
    variable = v
    value = 0
    penalty = ${penalty}
  []
  [u_walls]
    type = ADPenaltyDirichletBC
    boundary = 'bottom top'
    variable = u
    value = 0
    penalty = ${penalty}
  []
  [v_walls]
    type = ADPenaltyDirichletBC
    boundary = 'bottom top'
    variable = v
    value = 0
    penalty = ${penalty}
  []
  [temperature_in]
    type = ADPenaltyDirichletBC
    boundary = left
    variable = temperature
    value = 1
    penalty = ${penalty}
  []
  [momentum_x_advection_in]
    type = ADConservativeAdvectionBC
    boundary = left
    variable = u
    velocity_function = v_inlet
    primal_dirichlet_value = u_inlet
    primal_coefficient = rho
  []
  [momentum_y_advection_in]
    type = ADConservativeAdvectionBC
    boundary = left
    variable = v
    velocity_function = v_inlet
    primal_dirichlet_value = 0
    primal_coefficient = rho
  []
  [mass_in]
    type = ADConservativeAdvectionBC
    boundary = left
    variable = pressure
    velocity_function = v_inlet
    advected_quantity = -1
  []
  [temperature_advection_in]
    type = ADConservativeAdvectionBC
    boundary = left
    variable = temperature
    velocity_function = v_inlet
    primal_dirichlet_value = 1
  []
  [momentum_x_advection_out]
    type = ADConservativeAdvectionBC
    boundary = right
    variable = u
    velocity_mat_prop = velocity
    advected_quantity = rhou
  []
  [momentum_y_advection_out]
    type = ADConservativeAdvectionBC
    boundary = right
    variable = v
    velocity_mat_prop = velocity
    advected_quantity = rhov
  []
  [mass_out]
    type = ADConservativeAdvectionBC
    boundary = right
    variable = pressure
    velocity_mat_prop = velocity
    advected_quantity = -1
  []
  [temperature_advection_out]
    type = ADConservativeAdvectionBC
    boundary = right
    variable = temperature
    velocity_mat_prop = velocity
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = rho
    prop_values = ${rho}
  []
  [const_reg]
    type = GenericConstantMaterial
    prop_names = mu
    prop_values = ${mu}
  []
  [velocity]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = velocity
    u = u
    v = v
  []
  [rhou]
    type = ADParsedMaterial
    property_name = rhou
    coupled_variables = u
    material_property_names = rho
    expression = 'rho*u'
  []
  [rhov]
    type = ADParsedMaterial
    property_name = rhov
    coupled_variables = v
    material_property_names = rho
    expression = 'rho*v'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  nl_rel_tol = 1e-14
[]

[Postprocessors]
  [outlet_velocity]
    type = SideAverageValue
    boundary = right
    variable = u
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

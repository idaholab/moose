!include mass-energy-conservation-stokes.i

[Kernels]
  [momentum_x_convection]
    type = ADConservativeAdvection
    variable = u
    velocity_material = 'velocity'
    advected_quantity = 'rhou'
  []
  [momentum_y_convection]
    type = ADConservativeAdvection
    variable = v
    velocity_material = 'velocity'
    advected_quantity = 'rhov'
  []
  [temperature_convection]
    type = ADConservativeAdvection
    variable = temperature
    velocity_material = 'velocity'
  []
[]

[DGKernels]
  [momentum_x_convection]
    type = ADDGAdvection
    variable = u
    velocity = 'velocity'
    advected_quantity = 'rhou'
  []
  [momentum_y_convection]
    type = ADDGAdvection
    variable = v
    velocity = 'velocity'
    advected_quantity = 'rhov'
  []
  [temperature_convection]
    type = ADDGAdvection
    variable = temperature
    velocity = 'velocity'
  []
[]

[BCs]
  [advection_momentum_x_in]
    type = ADConservativeAdvectionBC
    boundary = 'left'
    variable = u
    velocity_function = v_inlet
    primal_dirichlet_value = 1
    primal_coefficient = 'rho'
  []
  [advection_momentum_y_in]
    type = ADConservativeAdvectionBC
    boundary = 'left'
    variable = v
    velocity_function = v_inlet
    primal_dirichlet_value = 0
    primal_coefficient = 'rho'
  []
  [advection_temperature_in]
    type = ADConservativeAdvectionBC
    boundary = 'left'
    variable = temperature
    velocity_function = v_inlet
    primal_dirichlet_value = 1
  []
  [advection_momentum_x_out]
    type = ADConservativeAdvectionBC
    boundary = 'right'
    variable = u
    velocity_mat_prop = 'velocity'
    advected_quantity = 'rhou'
  []
  [advection_momentum_y_out]
    type = ADConservativeAdvectionBC
    boundary = 'right'
    variable = v
    velocity_mat_prop = 'velocity'
    advected_quantity = 'rhov'
  []
  [advection_temperature_out]
    type = ADConservativeAdvectionBC
    boundary = 'right'
    variable = temperature
    velocity_mat_prop = 'velocity'
  []
[]

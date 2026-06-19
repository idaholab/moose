!include by-parts-channel-hybrid-stokes.i

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
[]

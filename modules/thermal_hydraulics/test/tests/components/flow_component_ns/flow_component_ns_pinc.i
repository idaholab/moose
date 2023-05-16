# Operating conditions
u_inlet = 1
p_outlet = 0

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
  [velocity_norm]
    type = MooseVariableFVReal
  []
[]

[Components]
  [comp1]
    type = FlowComponentNS
    file = rectangle.e
    position = '0 0 0'

    compressibility = 'incompressible'
    porous_medium_treatment = true

    density = 'rho'
    dynamic_viscosity = 'mu'

    initial_velocity = '${u_inlet} 0 0'
    initial_pressure = '${p_outlet}'

    mass_advection_interpolation = 'upwind'
    momentum_advection_interpolation = 'upwind'

    inlet_boundaries = 'comp1:left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '${u_inlet} 0'

    wall_boundaries = 'comp1:top comp1:bottom'
    momentum_wall_types = 'noslip symmetry'

    outlet_boundaries = 'comp1:right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '${p_outlet}'
  []

  [comp2]
    type = FlowComponentNS
    file = rectangle.e
    position = '0 2 0'

    compressibility = 'incompressible'
    porous_medium_treatment = true

    density = 'rho'
    dynamic_viscosity = 'mu'

    initial_velocity = '${u_inlet} 0 0'
    initial_pressure = '${p_outlet}'

    inlet_boundaries = 'comp2:left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '${u_inlet} 0'

    wall_boundaries = 'comp2:top comp2:bottom'
    momentum_wall_types = 'noslip symmetry'

    outlet_boundaries = 'comp2:right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '${p_outlet}'

    mass_advection_interpolation = 'average'
    momentum_advection_interpolation = 'average'
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '1  1'
  []
[]

[AuxKernels]
  [speed]
    type = ParsedAux
    variable = 'velocity_norm'
    coupled_variables = 'superficial_vel_x superficial_vel_y porosity'
    expression = 'sqrt(superficial_vel_x*superficial_vel_x + superficial_vel_y*superficial_vel_y) / porosity'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  line_search = 'none'
  nl_rel_tol = 1e-12
  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
  verbose = true
  scaling_group_variables = 'superficial_vel_x superficial_vel_y'
[]

[Debug]
  show_var_residual_norms = true
[]

# Some basic Postprocessors to examine the solution
[Postprocessors]
  [inlet-p]
    type = SideAverageValue
    variable = pressure
    boundary = 'comp1:left'
  []
  [outlet-u]
    type = SideAverageValue
    variable = superficial_vel_x
    boundary = 'comp1:right'
  []
[]

[Outputs]
  exodus = true
[]

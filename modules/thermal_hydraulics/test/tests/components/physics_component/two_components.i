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

[Functions]
  [f1]
    type = ConstantFunction
    value = 1
  []
[]

[Components]
  [comp1]
    type = FileMeshWCNSFVComponent
    file = rectangle.e
    position = '0 0 0'
    verbose = true

    add_flow_equations = true
    add_energy_equation = false
    add_scalar_equations = false

    inlet_boundaries = 'comp1:left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = 'f1 f1'

    wall_boundaries = 'comp1:top comp1:bottom'
    momentum_wall_types = 'noslip symmetry'

    outlet_boundaries = 'comp1:right'
    # momentum_outlet_types = 'fixed-pressure'
    # pressure_function = 'f1'

    initial_velocity = '${u_inlet} 0 0'
    initial_pressure = '${p_outlet}'

    mass_advection_interpolation = 'upwind'
    momentum_advection_interpolation = 'upwind'

    # The junction adds more boundary conditions
    boundary_conditions_all_set = false
  []
  [join]
    type = FileMeshWCNSFVFlowJunction
    connections = 'comp1:right:out comp2:left:in'
    junction_techniques = 'stitching boundary_values'
  []
  [comp2]
    type = FileMeshWCNSFVComponent
    file = rectangle.e
    position = '10 0 0'
    verbose = true

    add_flow_equations = true
    add_energy_equation = false
    add_scalar_equations = false

    # Rename the variables for now
    # We may want to prefix variable names later
    # We will have to connect all the fields
    velocity_variable = 'u v'
    pressure_variable = 'p'

    inlet_boundaries = 'comp2:left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = 'f1 f1'

    wall_boundaries = 'comp2:top comp2:bottom'
    momentum_wall_types = 'noslip symmetry'

    outlet_boundaries = 'comp2:right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = 'f1'

    initial_velocity = '${u_inlet} 0 0'
    initial_pressure = '${p_outlet}'

    mass_advection_interpolation = 'upwind'
    momentum_advection_interpolation = 'upwind'

    # The junction adds more boundary conditions
    boundary_conditions_all_set = false
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu total_viscosity'
    prop_values = '1  1 1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-5
  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
  verbose = true
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
  # [outlet-u]
  #   type = SideAverageValue
  #   variable = superficial_vel_x
  #   boundary = 'comp1:right'
  # []
[]

[Outputs]
  exodus = true
[]

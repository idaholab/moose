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

[Physics]
  [NavierStokes]
    [WCNSFVFlowPhysics]
      [flow]
        inlet_boundaries = 'comp1:left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = 'f1 f1'

        wall_boundaries = 'comp1:top comp1:bottom comp2:top comp2:bottom'
        momentum_wall_types = 'noslip symmetry noslip symmetry'

        outlet_boundaries = 'comp2:right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = 'f1'

        initial_velocity = '${u_inlet} 0 0'
        initial_pressure = '${p_outlet}'

        mass_advection_interpolation = 'upwind'
        momentum_advection_interpolation = 'upwind'

        # The junction adds more boundary conditions
        boundary_conditions_all_set = false
      []
    []
  []
[]

[Components]
  [comp1]
    type = FileMeshPhysicsComponent
    file = rectangle.e
    position = '0 0 0'

    physics = 'flow'
    incoming_boundaries = 'comp1:right'
    outgoing_boundaries = 'comp1:left'
  []
  [join]
    type = FileMeshStitchJunction
    connections = 'comp1:right:out comp2:left:in'
  []
  [comp2]
    type = FileMeshPhysicsComponent
    file = rectangle.e
    position = '10 0 0'

    physics = 'flow'
    incoming_boundaries = 'comp2:right'
    outgoing_boundaries = 'comp2:left'
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu k cp'
    prop_values = '1  1 1 1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type -ksp_view_pmat'
  petsc_options_value = 'lu       NONZERO               mumps                      ::ascii_matlab'
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
  [mid-p2]
    type = SideAverageValue
    variable = pressure
    boundary = 'comp2:left'
  []
[]

[Outputs]
  exodus = true
[]

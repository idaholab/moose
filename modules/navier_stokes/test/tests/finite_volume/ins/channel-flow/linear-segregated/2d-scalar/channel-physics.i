mu = 2.6
rho = 1.0
advected_interp_method = 'upwind'
k1 = 0.1
k2 = 0.2

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.25 0.25'
    dy = '0.2'
    ix = '5 5'
    iy = '5'
    subdomain_id = '0 1'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system s1_system s2_system'
[]


[Physics]
  [NavierStokes]
    [FlowSegregated/flow]
      velocity_variable = 'vel_x vel_y'
      pressure_variable = 'pressure'

      initial_velocity = '0.5 0 0'
      initial_pressure = '0.2'

      density = ${rho}
      dynamic_viscosity = ${mu}

      # use inlet for moving wall to match the reference input
      # we could also use a noslip BC with a velocity wall functor
      inlet_boundaries = 'left'
      momentum_inlet_types = 'fixed-velocity'
      momentum_inlet_functors = '1.1 0'

      wall_boundaries = 'top bottom'
      momentum_wall_types = 'noslip noslip'

      outlet_boundaries = 'right'
      momentum_outlet_types = 'fixed-pressure'
      pressure_functors = '1.4'

      orthogonality_correction = false
      pressure_two_term_bc_expansion = false
      momentum_advection_interpolation = ${advected_interp_method}
    []
    [ScalarTransportSegregated/scalar]
      passive_scalar_names = 'scalar1 scalar2'
      system_names = 's1_system s2_system'
      initial_scalar_variables = '1.1 3'

      passive_scalar_diffusivity = '${k1} ${k2}'

      passive_scalar_inlet_types = 'fixed-value fixed-value'
      passive_scalar_inlet_function = '1; 2'

      passive_scalar_advection_interpolation = ${advected_interp_method}
      passive_scalar_two_term_bc_expansion = false
      use_nonorthogonal_correction = false
    []
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  passive_scalar_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  passive_scalar_l_tol = 0
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  passive_scalar_systems = 's1_system s2_system'
  momentum_equation_relaxation = 0.8
  passive_scalar_equation_relaxation = '0.9 0.9'
  pressure_variable_relaxation = 0.3
  # We need to converge the problem to show conservation
  num_iterations = 200
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  passive_scalar_absolute_tolerance = '1e-10 1e-10'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  passive_scalar_petsc_options_iname = '-pc_type -pc_hypre_type'
  passive_scalar_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
  hide = 'pressure vel_x vel_y'
[]

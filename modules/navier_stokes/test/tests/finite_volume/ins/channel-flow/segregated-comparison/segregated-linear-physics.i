mu = 2.6
rho = 1.0
advected_interp_method = 'average'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.3'
    dy = '0.3'
    ix = '3'
    iy = '3'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
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

      # we use an inlet to mimick the original input
      # we could use a wall noslip boundary
      inlet_boundaries = 'left'
      momentum_inlet_types = 'fixed-velocity'
      momentum_inlet_functors = '1.1 0'

      wall_boundaries = 'top bottom'
      momentum_wall_types = 'noslip noslip'
      momentum_wall_functors = '0 0; 0 0'

      outlet_boundaries = 'right'
      momentum_outlet_types = 'fixed-pressure-zero-gradient'
      pressure_functors = '1.4'

      orthogonality_correction = false
      momentum_two_term_bc_expansion = false
      pressure_two_term_bc_expansion = false
      momentum_advection_interpolation = ${advected_interp_method}
    []
  []
[]

# Note: this executioner should be moved into the Physics
[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-10
  pressure_l_abs_tol = 1e-10
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  num_iterations = 2
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
[]

[Outputs]
  exodus = true
  execute_on = FINAL
[]

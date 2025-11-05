mu = .01
rho = 1
advected_interp_method = 'average'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = .1
    ymin = 0
    ymax = .1
    nx = 3
    ny = 3
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

      initial_velocity = '0 0 0'
      initial_pressure = '0.2'

      density = ${rho}
      dynamic_viscosity = ${mu}

      # use inlet for moving wall to match the reference input
      # we could also use a noslip BC with a velocity wall functor
      inlet_boundaries = 'top'
      momentum_inlet_types = 'fixed-velocity'
      momentum_inlet_functors = '1 0'

      wall_boundaries = 'left right bottom'
      momentum_wall_types = 'noslip noslip noslip'

      orthogonality_correction = false
      pressure_two_term_bc_expansion = true
      momentum_advection_interpolation = ${advected_interp_method}
    []
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  num_iterations = 100
  pressure_absolute_tolerance = 1e-12
  momentum_absolute_tolerance = 1e-12
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.05 0.05 0.0'
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]

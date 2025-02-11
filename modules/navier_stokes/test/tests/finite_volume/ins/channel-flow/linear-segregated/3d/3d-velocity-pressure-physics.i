mu = 2.6
rho = 1.0
advected_interp_method = 'average'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 3
    dx = '0.3'
    dy = '0.3'
    dz = '0.3'
    ix = '3'
    iy = '3'
    iz = '3'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system w_system pressure_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [FlowSegregated/flow]
      velocity_variable = 'vel_x vel_y vel_z'
      pressure_variable = 'pressure'

      initial_velocity = '0.5 0 0'
      initial_pressure = '0.2'

      density = ${rho}
      dynamic_viscosity = ${mu}

      # use inlet for moving wall to match the reference input
      # we could also use a noslip BC with a velocity wall functor
      inlet_boundaries = 'left'
      momentum_inlet_types = 'fixed-velocity'
      momentum_inlet_functors = '1.1 0 0'

      wall_boundaries = 'top bottom back front'
      momentum_wall_types = 'noslip noslip noslip noslip'

      outlet_boundaries = 'right'
      momentum_outlet_types = 'fixed-pressure'
      pressure_functors = '1.4'

      orthogonality_correction = false
      pressure_two_term_bc_expansion = false
      momentum_two_term_bc_expansion = false
      momentum_advection_interpolation = ${advected_interp_method}
    []
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-10
  pressure_l_abs_tol = 1e-10
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system w_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  num_iterations = 100
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type'
  momentum_petsc_options_value = 'hypre boomeramg 4 1 0.1 0.6 HMIS ext+i'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type'
  pressure_petsc_options_value = 'hypre boomeramg 2 1 0.1 0.6 HMIS ext+i'
  print_fields = false
[]

[Outputs]
  exodus = true
[]

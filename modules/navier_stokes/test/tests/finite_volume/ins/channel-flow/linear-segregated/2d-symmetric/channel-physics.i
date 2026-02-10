mu = 2.6
rho = 1.2
advected_interp_method = 'upwind'
u_inlet = 1.1
half_width = 0.2

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.0'
    dy = '${half_width}'
    ix = '30'
    iy = '15'
    subdomain_id = '0'
  []
  allow_renumbering = false
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
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

      inlet_boundaries = 'left'
      momentum_inlet_types = 'fixed-velocity'
      momentum_inlet_functors = '${u_inlet} 0'

      wall_boundaries = 'top bottom'
      momentum_wall_types = 'noslip symmetry'

      outlet_boundaries = 'right'
      momentum_outlet_types = 'fixed-pressure'
      pressure_functors = '1.4'

      orthogonality_correction = false
      pressure_two_term_bc_expansion = true
      momentum_two_term_bc_expansion = true
      momentum_advection_interpolation = ${advected_interp_method}
    []
  []
[]


[Functions]
  [u_parabolic_profile]
    type = ParsedFunction
    expression = '3/2*${u_inlet}*(1 - pow(y/${half_width}, 2))' # Poiseuille profile
  []
  [u_parabolic_profile_rz]
    type = ParsedFunction
    expression = '2*${u_inlet}*(1 - pow(y/${half_width}, 2))' # Cylindrical profile
  []
[]

[AuxVariables]
  [vel_exact]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [assign_vel_exact]
    type = FunctionAux
    variable = vel_exact
    function = u_parabolic_profile
    execute_on = TIMESTEP_END
  []
[]

[VectorPostprocessors]
  [outlet_velocity_profile]
    type = SideValueSampler
    variable = 'vel_x vel_exact'
    boundary = 'right'
    sort_by = 'y'
    execute_on = TIMESTEP_END
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
  momentum_equation_relaxation = 0.5
  pressure_variable_relaxation = 0.3
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]

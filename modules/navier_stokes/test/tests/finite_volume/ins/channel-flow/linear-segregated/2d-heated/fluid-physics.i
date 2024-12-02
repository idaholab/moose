mu = 2.6
rho = 1.0
advected_interp_method = 'upwind'
cp = 1000
k = 1
h_cv = 100

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
  linear_sys_names = 'u_system v_system pressure_system energy_system'
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
      momentum_inlet_functors = '1.1 0'

      wall_boundaries = 'top bottom'
      momentum_wall_types = 'noslip noslip'

      outlet_boundaries = 'right'
      momentum_outlet_types = 'fixed-pressure'
      pressure_functors = '1.4'

      orthogonality_correction = false
      pressure_two_term_bc_expansion = false
      momentum_two_term_bc_expansion = false
      momentum_advection_interpolation = ${advected_interp_method}
    []
    [FluidHeatTransferSegregated/energy]
      thermal_conductivity = ${k}
      specific_heat = ${cp}

      initial_temperature = 300

      # Left is an inlet
      energy_inlet_types = 'fixed-temperature'
      energy_inlet_functors = '300'

      # Top and bottom are fixed temperature 'inlets' in the reference kernel-based input
      energy_wall_types = 'fixed-temperature fixed-temperature'
      energy_wall_functors = '300 300'

      external_heat_source = 0

      ambient_convection_alpha = ${h_cv}
      ambient_temperature = 'T_solid'
      ambient_convection_blocks = '1'

      energy_advection_interpolation = ${advected_interp_method}
      energy_two_term_bc_expansion = false
      use_nonorthogonal_correction = false
    []
  []
[]

[AuxVariables]
  [T_solid]
    type = MooseLinearVariableFVReal
    initial_condition = 300
    block = 1
  []
[]

[MultiApps]
  inactive = 'solid'
  [solid]
    type = FullSolveMultiApp
    input_files = solid.i
    execute_on = timestep_begin
    no_restore = true
  []
[]

[Transfers]
  inactive = 'from_solid to_solid'
  [from_solid]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = solid
    source_variable = 'T_solid'
    variable = 'T_solid'
    execute_on = timestep_begin
    from_blocks = 1
  []
  [to_solid]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = solid
    source_variable = 'T_fluid'
    variable = 'T_fluid'
    execute_on = timestep_begin
    to_blocks = 1
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  energy_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.8
  energy_equation_relaxation = 0.9
  pressure_variable_relaxation = 0.3
  num_iterations = 20
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  energy_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]

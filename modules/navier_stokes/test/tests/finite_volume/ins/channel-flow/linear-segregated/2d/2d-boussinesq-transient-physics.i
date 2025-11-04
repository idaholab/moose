mu = 2.6
rho = 1.0
advected_interp_method = 'average'
cp = 300
k = 10
alpha_b = 1e-4

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.'
    dy = '0.2'
    ix = '10'
    iy = '5'
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
      fluid_temperature_variable = 'T'

      initial_velocity = '0.5 0 0'
      initial_pressure = '0.2'

      density = ${rho}
      dynamic_viscosity = ${mu}

      # To match non-physics-setup test
      solve_for_dynamic_pressure = true

      boussinesq_approximation = true
      ref_temperature = 300
      gravity = '0 -9.81 0'
      thermal_expansion = 'alpha_b'

      # use inlet for moving wall to match the reference input
      # we could also use a noslip BC with a velocity wall functor
      inlet_boundaries = 'left'
      momentum_inlet_types = 'fixed-velocity'
      momentum_inlet_functors = '1.1 0'

      outlet_boundaries = 'right'
      momentum_outlet_types = 'fixed-pressure'
      pressure_functors = '1.4'

      wall_boundaries = 'bottom top'
      momentum_wall_types = 'noslip noslip'

      orthogonality_correction = false
      momentum_two_term_bc_expansion = false
      pressure_two_term_bc_expansion = false
      momentum_advection_interpolation = ${advected_interp_method}
    []
    [FluidHeatTransferSegregated/energy]
      fluid_temperature_variable = 'T'
      thermal_conductivity = ${k}
      specific_heat = ${cp}

      initial_temperature = 300

      # Left is an inlet
      energy_inlet_types = 'fixed-temperature'
      energy_inlet_functors = '300'

      # Top and bottom are fixed temperature 'inlets' in the reference kernel-based input
      energy_wall_types = 'fixed-temperature fixed-temperature'
      energy_wall_functors = 'wall-temperature 300'

      energy_advection_interpolation = ${advected_interp_method}
      energy_two_term_bc_expansion = false
      use_nonorthogonal_correction = false
    []
  []
[]

[FunctorMaterials]
  [constant_functors]
    type = GenericFunctorMaterial
    prop_names = 'cp alpha_b rho_cp'
    prop_values = '${cp} ${alpha_b} ${fparse rho * cp}'
  []
[]

[Functions]
  [wall-temperature]
    type = ParsedFunction
    expression = '350 + 50 * sin(6.28*t)'
  []
[]

[Executioner]
  type = PIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  energy_l_abs_tol = 1e-12
  momentum_l_tol = 1e-12
  pressure_l_tol = 1e-12
  energy_l_tol = 1e-12
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.9
  num_iterations = 100
  pressure_absolute_tolerance = 1e-11
  momentum_absolute_tolerance = 1e-11
  energy_absolute_tolerance = 1e-11
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
  dt = 0.01
  num_steps = 6
  num_piso_iterations = 0
[]


[Outputs]
  exodus = true
[]

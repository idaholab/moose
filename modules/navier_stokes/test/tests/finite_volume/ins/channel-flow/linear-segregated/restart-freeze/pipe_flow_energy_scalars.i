rho = 1.1
mu = 0.15
cp = 1.2
k = 0.1
scalar_diff = 0.01
advected_interp_method = 'upwind'
pressure_outlet = 42
inlet_velocity = 0.6
inlet_temperature = 320
inlet_scalar = 13

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 1
    nx = 10
  []
[]

[Problem]
  linear_sys_names = 'u_system pressure_system energy_system scalar_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [FlowSegregated/flow]
      velocity_variable = 'vel_x'
      pressure_variable = 'pressure'

      density = ${rho}
      dynamic_viscosity = ${mu}

      inlet_boundaries = 'left'
      momentum_inlet_types = 'fixed-velocity'
      momentum_inlet_functors = ${inlet_velocity}

      outlet_boundaries = 'right'
      momentum_outlet_types = 'fixed-pressure'
      pressure_functors = ${pressure_outlet}

      orthogonality_correction = false
      pressure_two_term_bc_expansion = false
      momentum_two_term_bc_expansion = false
      momentum_advection_interpolation = ${advected_interp_method}
    []

    [FluidHeatTransferSegregated/energy]
      thermal_conductivity = ${k}
      specific_heat = ${cp}

      energy_inlet_types = 'fixed-temperature'
      energy_inlet_functors = ${inlet_temperature}

      energy_advection_interpolation = ${advected_interp_method}
      use_nonorthogonal_correction = false
      energy_two_term_bc_expansion = false
    []

    [ScalarTransportSegregated/scalar]
      passive_scalar_names = 'scalar'
      system_names = 'scalar_system'

      passive_scalar_diffusivity = '${scalar_diff}'

      passive_scalar_inlet_types = 'fixed-value'
      passive_scalar_inlet_functors = ${inlet_scalar}

      passive_scalar_advection_interpolation = ${advected_interp_method}
      passive_scalar_two_term_bc_expansion = false
      use_nonorthogonal_correction = false
    []
  []
[]

[Executioner]
  type = SIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  passive_scalar_systems = 'scalar_system'

  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  energy_l_abs_tol = 1e-12
  passive_scalar_l_abs_tol = 1e-12

  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  passive_scalar_l_tol = 0

  momentum_equation_relaxation = 0.7
  energy_equation_relaxation = 1.0
  passive_scalar_equation_relaxation = '1.0'
  pressure_variable_relaxation = 0.3

  num_iterations = 150
  pressure_absolute_tolerance = 1e-11
  momentum_absolute_tolerance = 1e-11
  energy_absolute_tolerance = 1e-11
  passive_scalar_absolute_tolerance = '1e-11'

  # We are going with LU because it is a 1D problem with 10 cells
  momentum_petsc_options_iname = '-pc_type'
  momentum_petsc_options_value = 'lu'
  pressure_petsc_options_iname = '-pc_type'
  pressure_petsc_options_value = 'lu'
  energy_petsc_options_iname = '-pc_type'
  energy_petsc_options_value = 'lu'
  passive_scalar_petsc_options_iname = '-pc_type'
  passive_scalar_petsc_options_value = 'lu'
  continue_on_max_its = true
[]

[VectorPostprocessors]
  [line_sampler]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 10
    sort_by = x
    variable = 'vel_x pressure T_fluid scalar'
    execute_on = FINAL
  []
[]

[Outputs]
  csv = true
  execute_on = final
[]

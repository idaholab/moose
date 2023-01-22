[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '5 5 5 10 5'
    ix = '5 5 5 10 5'
    dy = '5 10 5 5 5'
    iy = '5 10 5 5 5'
    subdomain_id = '2 3 2 2 2
                    2 1 2 2 2
                    2 1 2 2 2
                    2 1 1 1 4
                    2 2 2 2 2'
  []

  [attached_to_fluid_block_inlet]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 3
    input = gen
    new_boundary = attached_to_fluid_block_inlet
  []

  [attached_to_fluid_block_outlet]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 4
    input = attached_to_fluid_block_inlet
    new_boundary = attached_to_fluid_block_outlet
  []

  [attached_to_fluid_block_walls]
    type = SideSetsBetweenSubdomainsGenerator
    input = attached_to_fluid_block_outlet
    primary_block = '1'
    paired_block = '2'
    new_boundary = attached_to_fluid_block_walls
  []

  [attached_to_non_fluid_block_inlet]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 3
    paired_block = 1
    input = attached_to_fluid_block_walls
    new_boundary = attached_to_non_fluid_block_inlet
  []

  [attached_to_non_fluid_block_outlet]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 4
    paired_block = 1
    input = attached_to_non_fluid_block_inlet
    new_boundary = attached_to_non_fluid_block_outlet
  []

  [attached_to_non_fluid_block_walls]
    type = SideSetsBetweenSubdomainsGenerator
    input = attached_to_non_fluid_block_outlet
    primary_block = '2'
    paired_block = '1'
    new_boundary = attached_to_non_fluid_block_walls
  []
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Modules]
  [NavierStokesFV]
    block = 1
    compressibility = 'incompressible'

    pressure_face_interpolation = average
    momentum_advection_interpolation = upwind
    mass_advection_interpolation = upwind

    # fluid properties
    density = '1'
    dynamic_viscosity = '1e-2'

    # initial conditions
    initial_velocity = '0 0 0'
    initial_pressure = 0

    # boundary conditions
    inlet_boundaries = 'attached_to_non_fluid_block_inlet'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '0 0.1'

    outlet_boundaries = 'attached_to_non_fluid_block_outlet'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'

    wall_boundaries = 'attached_to_non_fluid_block_walls'
    momentum_wall_types = 'slip'
  []
[]

[Postprocessors]
  [outlet_mfr]
    type = VolumetricFlowRate
    vel_x = 'vel_x'
    vel_y = 'vel_y'
    advected_quantity = 1
    boundary = attached_to_fluid_block_outlet
    rhie_chow_user_object = ins_rhie_chow_interpolator
  []

  [inlet_mfr]
    type = VolumetricFlowRate
    vel_x = 'vel_x'
    vel_y = 'vel_y'
    advected_quantity = 1
    boundary = attached_to_fluid_block_inlet
    rhie_chow_user_object = ins_rhie_chow_interpolator
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options = '-pc_svd_monitor'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'svd'
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.1
    growth_factor = 1.1
    cutback_factor = 0.9
    optimal_iterations = 6
    iteration_window = 2
  []
  nl_max_its = 10
  nl_abs_tol = 1e-5
  # Steady state detection.
  steady_state_detection = true
  steady_state_tolerance = 1e-10
  num_steps = 1
[]

[Outputs]
  exodus = true
[]

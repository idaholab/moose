# natural convection through a dogleg
height = 2.2
density = 1.2
gravity = 10
head = ${fparse height * density * gravity}

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 0.2 0.2 0.2 1'
    ix = '1 3   3   3   1'
    dy = '1   0.2 1'
    iy = '12  3   12'
    subdomain_id = '2 1 2 2 3
                    2 1 1 1 3
                    2 2 2 1 3'
  []

  [wall]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '1'
    paired_block = '2'
    new_boundary = wall
  []

  [heated_wall]
    type = SideSetsBetweenSubdomainsGenerator
    input = wall
    primary_block = '1'
    paired_block = '3'
    new_boundary = heated_wall
  []

  [delete]
    type = BlockDeletionGenerator
    block = '2 3'
    input = heated_wall
  []
[]

[GlobalParams]
  rhie_chow_user_object = ins_rhie_chow_interpolator
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    add_energy_equation = true
    boussinesq_approximation = true
    thermal_expansion = 8e-4
    gravity = '0 -${gravity} 0'
    density = 1.2
    dynamic_viscosity = 1e-3
    specific_heat = 300
    thermal_conductivity = '10'
    initial_velocity = '0 1e-6 0'
    initial_pressure = 0
    inlet_boundaries = 'bottom'
    momentum_inlet_types = 'fixed-pressure'
    momentum_inlet_function = '${head}'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = '300'
    wall_boundaries = 'wall heated_wall'
    momentum_wall_types = 'slip slip'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '0 300'
    outlet_boundaries = 'top'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'
    energy_advection_interpolation = 'average'
    momentum_advection_interpolation = 'upwind'
    mass_advection_interpolation = 'upwind'
    friction_blocks = '1'
    friction_types = 'Darcy'
    friction_coeffs = '2'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'lu        NONZERO'
  nl_rel_tol = 1e-8
[]

[Materials]
  [props]
    type = ADGenericFunctorMaterial
    prop_names = 'rho'
    prop_values = '${density}'
  []
[]

[Postprocessors]
  [inlet_mfr]
    type = VolumetricFlowRate
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = rho
    boundary = bottom
  []

  [outlet_mfr]
    type = VolumetricFlowRate
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = rho
    boundary = top
  []
[]

[Outputs]
  exodus = true
[]

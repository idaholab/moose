[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'ns-restart-steady_out.e'
    use_for_exodus_restart = true
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_from_file_var = vel_x
    initial_from_file_timestep = LATEST
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_from_file_var = vel_y
    initial_from_file_timestep = LATEST
  []
  [pressure]
    type = INSFVPressureVariable
    initial_from_file_var = pressure
    initial_from_file_timestep = LATEST
  []
  [T_fluid]
    type = INSFVEnergyVariable
    initial_from_file_var = T_fluid
    initial_from_file_timestep = LATEST
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    add_energy_equation = true

    density = 1
    dynamic_viscosity = 1
    thermal_conductivity = 1e-3
    specific_heat = 1

    velocity_variable = 'vel_x vel_y'
    pressure_variable = 'pressure'
    fluid_temperature_variable = 'T_fluid'

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '0.1 0'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = '1'

    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip noslip'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '0 0'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'

    ambient_convection_alpha = 1
    ambient_temperature = '100'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
  end_time = 2
  dt = 1
[]

[Outputs]
  exodus = true
[]

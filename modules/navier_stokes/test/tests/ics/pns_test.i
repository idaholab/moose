p_initial=1.01e5
T=273.15

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 1
    ymax = 2
    nx = 4
    ny = 4
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
  skip_nl_system_check = true
[]

[AuxVariables]
  [porosity]
    initial_condition = 0.2
  []
  [pressure]
    type = MooseVariableFVReal
  []
  [superficial_vel_x]
    type = MooseVariableFVReal
  []
  [superficial_vel_y]
    type = MooseVariableFVReal
  []
  [superficial_vel_z]
    type = MooseVariableFVReal
  []
  [temperature]
    type = MooseVariableFVReal
  []
  [vel_x]
    type = MooseVariableFVReal
  []
  [vel_y]
    type = MooseVariableFVReal
  []
  [vel_z]
    type = MooseVariableFVReal
  []
  [superficial_rho_ht]
    type = MooseVariableFVReal
  []
  [ht]
    type = MooseVariableFVReal
  []
  [e]
    type = MooseVariableFVReal
  []
  [Mach]
    type = MooseVariableFVReal
  []
  [superficial_rho]
    type = MooseVariableFVReal
  []
  [superficial_rhou]
    type = MooseVariableFVReal
  []
  [superficial_rhov]
    type = MooseVariableFVReal
  []
  [superficial_rhow]
    type = MooseVariableFVReal
  []
  [superficial_rho_et]
    type = MooseVariableFVReal
  []
  [rho]
    type = MooseVariableFVReal
  []
  [rhou]
    type = MooseVariableFVReal
  []
  [rhov]
    type = MooseVariableFVReal
  []
  [rhow]
    type = MooseVariableFVReal
  []
  [rho_et]
    type = MooseVariableFVReal
  []
  [specific_volume]
    type = MooseVariableFVReal
  []
  [pressure_2]
  []
  [superficial_vel_x_2]
  []
  [superficial_vel_y_2]
  []
  [superficial_vel_z_2]
  []
  [vel_x_2]
  []
  [vel_y_2]
  []
  [vel_z_2]
  []
  [temperature_2]
  []
  [ht_2]
  []
  [superficial_rho_ht_2]
  []
  [e_2]
  []
  [Mach_2]
  []
  [superficial_rho_2]
  []
  [superficial_rhou_2]
  []
  [superficial_rhov_2]
  []
  [superficial_rhow_2]
  []
  [superficial_rho_et_2]
  []
  [rho_2]
  []
  [rhou_2]
  []
  [rhov_2]
  []
  [rhow_2]
  []
  [rho_et_2]
  []
  [specific_volume_2]
  []
[]

[GlobalParams]
  fluid_properties = 'fp'
  initial_pressure = ${p_initial}
  initial_temperature = ${T}
  initial_superficial_velocity = '1 0.2 18'
  porosity = porosity
[]

[ICs]
  [p]
    type = PNSInitialCondition
    variable = 'pressure'
  []
  [vel_x]
    type = PNSInitialCondition
    variable = 'vel_x'
  []
  [vel_y]
    type = PNSInitialCondition
    variable = 'vel_y'
  []
  [vel_z]
    type = PNSInitialCondition
    variable = 'vel_z'
  []
  [superficial_vel_x]
    type = PNSInitialCondition
    variable = 'superficial_vel_x'
  []
  [superficial_vel_y]
    type = PNSInitialCondition
    variable = 'superficial_vel_y'
  []
  [superficial_vel_z]
    type = PNSInitialCondition
    variable = 'superficial_vel_z'
  []
  [temperature]
    type = PNSInitialCondition
    variable = 'temperature'
  []
  [ht]
    type = PNSInitialCondition
    variable = 'ht'
  []
  [superficial_rho_ht]
    type = PNSInitialCondition
    variable = 'superficial_rho_ht'
  []
  [e]
    type = PNSInitialCondition
    variable = 'e'
  []
  [Mach]
    type = PNSInitialCondition
    variable = 'Mach'
  []
  [superficial_rho]
    type = PNSInitialCondition
    variable = 'superficial_rho'
  []
  [superficial_rhou]
    type = PNSInitialCondition
    fluid_properties = 'fp'
    initial_pressure = ${p_initial}
    initial_temperature = ${T}
    initial_superficial_velocity = '1 0.2 18'
    porosity = porosity
    variable = 'superficial_rhou'
  []
  [superficial_rhov]
    type = PNSInitialCondition
    variable = 'superficial_rhov'
  []
  [superficial_rhow]
    type = PNSInitialCondition
    variable = 'superficial_rhow'
  []
  [rho]
    type = PNSInitialCondition
    variable = 'rho'
  []
  [rhou]
    type = PNSInitialCondition
    variable = 'rhou'
  []
  [rhov]
    type = PNSInitialCondition
    variable = 'rhov'
  []
  [rhow]
    type = PNSInitialCondition
    variable = 'rhow'
  []
  [rho_et]
    type = PNSInitialCondition
    variable = 'rho_et'
  []
  [superficial_rho_et]
    type = PNSInitialCondition
    variable = 'superficial_rho_et'
  []
  [specific_volume]
    type = PNSInitialCondition
    variable = 'specific_volume'
  []
  [p_2]
    type = PNSInitialCondition
    variable = 'pressure_2'
    variable_type = 'pressure'
  []
  [superficial_vel_x_2]
    type = PNSInitialCondition
    variable = 'superficial_vel_x_2'
    variable_type = 'superficial_vel_x'
  []
  [superficial_vel_y_2]
    type = PNSInitialCondition
    variable = 'superficial_vel_y_2'
    variable_type = 'superficial_vel_y'
  []
  [superficial_vel_z_2]
    type = PNSInitialCondition
    variable = 'superficial_vel_z_2'
    variable_type = 'superficial_vel_z'
  []
  [vel_x_2]
    type = PNSInitialCondition
    variable = 'vel_x_2'
    variable_type = 'vel_x'
  []
  [vel_y_2]
    type = PNSInitialCondition
    variable = 'vel_y_2'
    variable_type = 'vel_y'
  []
  [vel_z_2]
    type = PNSInitialCondition
    variable = 'vel_z_2'
    variable_type = 'vel_z'
  []
  [temperature_2]
    type = PNSInitialCondition
    variable = 'temperature_2'
    variable_type = 'temperature'
  []
  [superficial_ht_2]
    type = PNSInitialCondition
    variable = 'superficial_rho_ht_2'
    variable_type = 'superficial_rho_ht'
  []
  [ht_2]
    type = PNSInitialCondition
    variable = 'ht_2'
    variable_type = 'ht'
  []
  [e_2]
    type = PNSInitialCondition
    variable = 'e_2'
    variable_type = 'e'
  []
  [Mach_2]
    type = PNSInitialCondition
    variable = 'Mach_2'
    variable_type = 'Mach'
  []
  [superficial_rho_2]
    type = PNSInitialCondition
    variable = 'superficial_rho_2'
    variable_type = 'superficial_rho'
  []
  [superficial_rhou_2]
    type = PNSInitialCondition
    variable = 'superficial_rhou_2'
    variable_type = 'superficial_rhou'
  []
  [superficial_rhov_2]
    type = PNSInitialCondition
    variable = 'superficial_rhov_2'
    variable_type = 'superficial_rhov'
  []
  [superficial_rhow_2]
    type = PNSInitialCondition
    variable = 'superficial_rhow_2'
    variable_type = 'superficial_rhow'
  []
  [superficial_rho_et_2]
    type = PNSInitialCondition
    variable = 'superficial_rho_et_2'
    variable_type = 'superficial_rho_et'
  []
  [rho_2]
    type = PNSInitialCondition
    variable = 'rho_2'
    variable_type = 'rho'
  []
  [rhou_2]
    type = PNSInitialCondition
    variable = 'rhou_2'
    variable_type = 'rhou'
  []
  [rhov_2]
    type = PNSInitialCondition
    variable = 'rhov_2'
    variable_type = 'rhov'
  []
  [rhow_2]
    type = PNSInitialCondition
    variable = 'rhow_2'
    variable_type = 'rhow'
  []
  [rho_et_2]
    type = PNSInitialCondition
    variable = 'rho_et_2'
    variable_type = 'rho_et'
  []
  [specific_volume_2]
    type = PNSInitialCondition
    variable = 'specific_volume_2'
    variable_type = 'specific_volume'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

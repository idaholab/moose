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
  [pressure]
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
  [temperature]
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
  [e_2]
  []
  [Mach_2]
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
  initial_velocity = '1 0.2 18'
[]

[ICs]
  [p]
    type = NSInitialCondition
    variable = 'pressure'
  []
  [vel_x]
    type = NSInitialCondition
    variable = 'vel_x'
  []
  [vel_y]
    type = NSInitialCondition
    variable = 'vel_y'
  []
  [vel_z]
    type = NSInitialCondition
    variable = 'vel_z'
  []
  [temperature]
    type = NSInitialCondition
    variable = 'temperature'
  []
  [ht]
    type = NSInitialCondition
    variable = 'ht'
  []
  [e]
    type = NSInitialCondition
    variable = 'e'
  []
  [Mach]
    type = NSInitialCondition
    variable = 'Mach'
  []
  [rho]
    type = NSInitialCondition
    fluid_properties = 'fp'
    initial_pressure = ${p_initial}
    initial_temperature = ${T}
    initial_velocity = '1 0.2 18'
    variable = 'rho'
  []
  [rhou]
    type = NSInitialCondition
    variable = 'rhou'
  []
  [rhov]
    type = NSInitialCondition
    variable = 'rhov'
  []
  [rhow]
    type = NSInitialCondition
    variable = 'rhow'
  []
  [rho_et]
    type = NSInitialCondition
    variable = 'rho_et'
  []
  [specific_volume]
    type = NSInitialCondition
    variable = 'specific_volume'
  []
  [p_2]
    type = NSInitialCondition
    variable = 'pressure_2'
    variable_type = 'pressure'
  []
  [vel_x_2]
    type = NSInitialCondition
    variable = 'vel_x_2'
    variable_type = 'vel_x'
  []
  [vel_y_2]
    type = NSInitialCondition
    variable = 'vel_y_2'
      variable_type = 'vel_y'
  []
  [vel_z_2]
    type = NSInitialCondition
    variable = 'vel_z_2'
      variable_type = 'vel_z'
  []
  [temperature_2]
    type = NSInitialCondition
    variable = 'temperature_2'
    variable_type = 'temperature'
  []
  [ht_2]
    type = NSInitialCondition
    variable = 'ht_2'
    variable_type = 'ht'
  []
  [e_2]
    type = NSInitialCondition
    variable = 'e_2'
    variable_type = 'e'
  []
  [Mach_2]
    type = NSInitialCondition
    variable = 'Mach_2'
    variable_type = 'Mach'
  []
  [rho_2]
    type = NSInitialCondition
    variable = 'rho_2'
    variable_type = 'rho'
  []
  [rhou_2]
    type = NSInitialCondition
    variable = 'rhou_2'
    variable_type = 'rhou'
  []
  [rhov_2]
    type = NSInitialCondition
    variable = 'rhov_2'
    variable_type = 'rhov'
  []
  [rhow_2]
    type = NSInitialCondition
    variable = 'rhow_2'
    variable_type = 'rhow'
  []
  [rho_et_2]
    type = NSInitialCondition
    variable = 'rho_et_2'
    variable_type = 'rho_et'
  []
  [specific_volume_2]
    type = NSInitialCondition
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

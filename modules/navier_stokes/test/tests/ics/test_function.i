[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 1
    ymax = 2
    nx = 3
    ny = 3
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
  initial_pressure = p_func
  initial_temperature = T_func
  initial_velocity = 'vx vy vz'
[]

[Functions]
  [p_func]
    type = ParsedFunction
    expression = '3+3+1e5 - x'
  []
  [T_func]
    type = ParsedFunction
    expression = '273.15 + x*y*2'
  []
  [vx]
    type = ParsedFunction
    expression = '14'
  []
  [vy]
    type = ParsedFunction
    expression = '10 + x'
  []
  [vz]
    type = ParsedFunction
    expression = '12 -7*y'
  []
[]

[ICs]
  [p]
    type = NSFunctionInitialCondition
    variable = 'pressure'
  []
  [vel_x]
    type = NSFunctionInitialCondition
    variable = 'vel_x'
  []
  [vel_y]
    type = NSFunctionInitialCondition
    variable = 'vel_y'
  []
  [vel_z]
    type = NSFunctionInitialCondition
    variable = 'vel_z'
  []
  [temperature]
    type = NSFunctionInitialCondition
    variable = 'temperature'
  []
  [ht]
    type = NSFunctionInitialCondition
    variable = 'ht'
  []
  [e]
    type = NSFunctionInitialCondition
    variable = 'e'
  []
  [Mach]
    type = NSFunctionInitialCondition
    variable = 'Mach'
  []
  [rho]
    type = NSFunctionInitialCondition
    fluid_properties = 'fp'
    initial_pressure = p_func
    initial_temperature = T_func
    initial_velocity = 'vx vy vz'
    variable = 'rho'
  []
  [rhou]
    type = NSFunctionInitialCondition
    variable = 'rhou'
  []
  [rhov]
    type = NSFunctionInitialCondition
    variable = 'rhov'
  []
  [rhow]
    type = NSFunctionInitialCondition
    variable = 'rhow'
  []
  [rho_et]
    type = NSFunctionInitialCondition
    variable = 'rho_et'
  []
  [specific_volume]
    type = NSFunctionInitialCondition
    variable = 'specific_volume'
  []
  [p_2]
    type = NSFunctionInitialCondition
    variable = 'pressure_2'
    variable_type = 'pressure'
  []
  [vel_x_2]
    type = NSFunctionInitialCondition
    variable = 'vel_x_2'
    variable_type = 'vel_x'
  []
  [vel_y_2]
    type = NSFunctionInitialCondition
    variable = 'vel_y_2'
      variable_type = 'vel_y'
  []
  [vel_z_2]
    type = NSFunctionInitialCondition
    variable = 'vel_z_2'
      variable_type = 'vel_z'
  []
  [temperature_2]
    type = NSFunctionInitialCondition
    variable = 'temperature_2'
    variable_type = 'temperature'
  []
  [ht_2]
    type = NSFunctionInitialCondition
    variable = 'ht_2'
    variable_type = 'ht'
  []
  [e_2]
    type = NSFunctionInitialCondition
    variable = 'e_2'
    variable_type = 'e'
  []
  [Mach_2]
    type = NSFunctionInitialCondition
    variable = 'Mach_2'
    variable_type = 'Mach'
  []
  [rho_2]
    type = NSFunctionInitialCondition
    variable = 'rho_2'
    variable_type = 'rho'
  []
  [rhou_2]
    type = NSFunctionInitialCondition
    variable = 'rhou_2'
    variable_type = 'rhou'
  []
  [rhov_2]
    type = NSFunctionInitialCondition
    variable = 'rhov_2'
    variable_type = 'rhov'
  []
  [rhow_2]
    type = NSFunctionInitialCondition
    variable = 'rhow_2'
    variable_type = 'rhow'
  []
  [rho_et_2]
    type = NSFunctionInitialCondition
    variable = 'rho_et_2'
    variable_type = 'rho_et'
  []
  [specific_volume_2]
    type = NSFunctionInitialCondition
    variable = 'specific_volume_2'
    variable_type = 'specific_volume'
  []
[]

[Executioner]
  type = Steady
[]

[Debug]
  show_actions = true
[]

[Outputs]
  exodus = true
[]

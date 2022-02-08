# Tests energy conservation for HeatStructureFromFile3D in combination with HeatSourceFromTotalPower

power = 1e5
power_fraction = 0.3
t = 1

energy_change = ${fparse power_fraction * power * t}

[Functions]
  [power_shape]
    type = ConstantFunction
    value = 0.4
  []
[]

[Materials]
  [mat]
    type = ADGenericConstantMaterial
    block = 'heat_structure:rgn1 heat_structure:rgn2'
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '100 500 1e4'
  []
[]

[Components]
  [heat_structure]
    type = HeatStructureFromFile3D
    file = box.e
    position = '0 0 0'
    initial_T = 300
  []
  [heat_generation]
    type = HeatSourceFromTotalPower
    hs = heat_structure
    regions = 'rgn1'
    power = total_power
    power_fraction = ${power_fraction}
  []
  [total_power]
    type = TotalPower
    power = ${power}
  []
[]

[Postprocessors]
  [E_tot]
    type = ADHeatStructureEnergy3D
    block = 'heat_structure:rgn1 heat_structure:rgn2'
    execute_on = 'initial timestep_end'
  []
  [E_tot_change]
    type = ChangeOverTimePostprocessor
    change_with_respect_to_initial = true
    postprocessor = E_tot
    execute_on = 'initial timestep_end'
  []

  [E_tot_change_rel_err]
    type = RelativeDifferencePostprocessor
    value1 = E_tot_change
    value2 = ${energy_change}
    execute_on = 'initial timestep_end'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10

  start_time = 0.0
  dt = ${t}
  num_steps = 1

  abort_on_solve_fail = true
[]

[Outputs]
  csv = true
  show = 'E_tot_change_rel_err'
  execute_on = 'final'
[]

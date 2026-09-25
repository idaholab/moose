# Tests that two cylindrical heat structure ActionComponents in one simulation correctly share the
# bare 'T_solid' variable name across their disjoint blocks (via THMVariableCoordinator), and that
# each independently conserves its own energy (they are not thermally coupled to each other).

[ActionComponents]
  [hs1]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 3
    names = 'FUEL'
    widths = '0.005'
    n_part_elems = '3'
    initial_T = '500 + 50*sin(x*pi)'
  []
  [hs2]
    type = HeatStructureCylindrical
    position = '2 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 3
    names = 'FUEL'
    widths = '0.005'
    n_part_elems = '3'
    initial_T = '700 + 50*sin(x*pi)'
  []
[]

[Materials]
  [mat]
    type = ADGenericConstantMaterial
    block = 'hs1:FUEL hs2:FUEL'
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '3.65 288.734 1.0412e2'
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
  start_time = 0
  dt = 1
  num_steps = 2
  solve_type = NEWTON
[]

[Postprocessors]
  [T1_avg]
    type = ElementAverageValue
    variable = T_solid
    block = hs1:FUEL
    execute_on = 'initial timestep_end'
  []
  [T2_avg]
    type = ElementAverageValue
    variable = T_solid
    block = hs2:FUEL
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]

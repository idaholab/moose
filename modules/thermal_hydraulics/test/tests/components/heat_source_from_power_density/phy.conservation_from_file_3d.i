t = 0.5

# these are the dimensions of rgn1 from box.e
width = 1.5
height = 5
depth = 2

density = 3
specific_heat_capacity = 1
conductivity = 5

power_density = 20

E_change = ${fparse power_density * width * height * depth * t}

[Functions]
  [power_density_fn]
    type = ConstantFunction
    value = ${power_density}
  []
[]

[AuxVariables]
  [power_density]
    family = MONOMIAL
    order = CONSTANT
    block = 'heat_structure:rgn1'
  []
[]

[AuxKernels]
  [mock_power_aux]
    type = FunctionAux
    variable = power_density
    function = power_density_fn
  []
[]

[Materials]
  [mat]
    type = ADGenericConstantMaterial
    block = 'heat_structure:rgn1 heat_structure:rgn2'
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '${density} ${specific_heat_capacity} ${conductivity}'
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
    type = HeatSourceFromPowerDensity
    hs = heat_structure
    regions = 'rgn1'
    power_density = power_density
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
    value2 = ${E_change}
    execute_on = 'INITIAL TIMESTEP_END'
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

  solve_type = 'newton'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10

  start_time = 0.0
  dt = 0.5
  num_steps = 1

  abort_on_solve_fail = true
[]

[Outputs]
  csv = true
  show = 'E_tot_change_rel_err'
  execute_on = 'final'
[]

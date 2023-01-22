# Tests that cylindrical heat structure thermal properties can be defined in the Materials block

[Functions]
  [power_profile_fn]
    type = ParsedFunction
    expression = '1.570796326794897 * sin(x / 3.6576 * pi)'
  []
[]

[SolidProperties]
  [function_sp]
    type = ThermalFunctionSolidProperties
    rho = 6.6e1
    cp = 321.384
    k = 16.48672
  []
[]

[Materials]
  [fuel-mat]
    type = ADGenericConstantMaterial
    block = hs:FUEL
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '3.65 288.734 1.0412e2'
  []

  [gap-mat]
    type = ADGenericConstantMaterial
    block = hs:GAP
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '1.084498 1.0 1.0'
  []

  [clad-mat]
    type = ADConstantDensityThermalSolidPropertiesMaterial
    block = hs:CLAD
    sp = function_sp
    temperature = T_solid
    T_ref = 300
  []
[]

[Components]
  [reactor]
    type = TotalPower
    power = 296153.84615384615385
  []

  [hs]
    type = HeatStructureCylindrical
    position = '0 0 1'
    orientation = '1 0 0'

    length = 3.6576
    n_elems = 20
    names = 'FUEL GAP CLAD'
    widths = '0.0046955  0.0000955  0.000673'
    n_part_elems = '3 1 1'

    initial_T = 564.15
  []

  [hg]
    type = HeatSourceFromTotalPower
    hs = hs
    regions = 'FUEL'
    power_fraction = 3.33672612e-1
    power = reactor
    power_shape_function = power_profile_fn
  []

  [temp_outside]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:outer
    T = 600
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

  start_time = 0
  dt = 2
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 30

  l_tol = 1e-4
  l_max_its = 300
[]


[Outputs]
  exodus = true
  [console]
    type = Console
    execute_scalars_on = none
  []
[]

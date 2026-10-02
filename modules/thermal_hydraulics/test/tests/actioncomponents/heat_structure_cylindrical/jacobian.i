[ActionComponents]
  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'

    length = 1.0
    n_elems = 3

    names = 'FUEL GAP CLAD'
    widths = '0.0046955 0.0000955 0.000673'
    n_part_elems = '2 1 1'

    initial_T = '564.15 + 50*sin(x*pi)'
  []
[]

[Materials]
  [fuel_mat]
    type = ADGenericConstantMaterial
    block = hs:FUEL
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '3.65 288.734 1.0412e2'
  []
  [gap_mat]
    type = ADGenericConstantMaterial
    block = hs:GAP
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '1.084498 1.0 1.0'
  []
  [clad_mat]
    type = ADGenericConstantMaterial
    block = hs:CLAD
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '16.0 321.384 6.6e1'
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
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
[]

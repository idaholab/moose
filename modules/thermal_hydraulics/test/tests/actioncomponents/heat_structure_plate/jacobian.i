[ActionComponents]
  [hs]
    type = HeatStructurePlate
    position = '0 0 0'
    orientation = '1 0 0'

    length = 1.0
    n_elems = 3
    depth = 0.1

    names = 'A B'
    widths = '0.01 0.02'
    n_part_elems = '2 2'

    initial_T = '300 + 50*sin(x*pi)'
  []
[]

[Materials]
  [a_mat]
    type = ADGenericConstantMaterial
    block = hs:A
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '10 500 8000'
  []
  [b_mat]
    type = ADGenericConstantMaterial
    block = hs:B
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '5 400 7000'
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

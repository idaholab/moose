# Tests that a plate heat structure ActionComponent, with a non-uniform initial temperature and
# insulated (default) boundaries, conducts heat correctly: the spatially-averaged temperature must
# stay exactly constant over time (pure conduction only redistributes energy), even as the spatial
# profile diffuses.

[ActionComponents]
  [hs]
    type = HeatStructurePlate
    position = '0 0 0'
    orientation = '1 0 0'

    length = 1.0
    n_elems = 5
    depth = 0.1

    names = 'A B'
    widths = '0.01 0.02'
    n_part_elems = '2 3'

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
  num_steps = 3
  solve_type = NEWTON
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
[]

[Postprocessors]
  [T_avg]
    type = ElementAverageValue
    variable = T_solid
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]

# Tests that a cylindrical heat structure ActionComponent, with a non-uniform initial temperature
# and insulated (default) boundaries, conducts heat correctly: the spatially-averaged temperature
# must stay exactly constant over time (pure conduction only redistributes energy, it cannot
# create or destroy it), even as the spatial profile diffuses.

[ActionComponents]
  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'

    length = 1.0
    n_elems = 5

    names = 'FUEL GAP CLAD'
    widths = '0.0046955 0.0000955 0.000673'
    n_part_elems = '3 1 1'

    initial_T = '564.15 + 100*sin(x*pi)'
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
  num_steps = 5
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

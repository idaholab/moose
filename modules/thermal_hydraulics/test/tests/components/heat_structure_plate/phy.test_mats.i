[Materials]
  [hs-mat]
    type = ADGenericConstantMaterial
    block = hs:blk
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '1 1 1'
  []
[]

[Components]
  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'

    length = 1
    n_elems = 5
    names = 'blk'
    widths = '1'
    n_part_elems = '5'

    initial_T = 350
  []

  [start]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:start
    T = 300
  []

  [end]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:end
    T = 400
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
  dt = 1
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]

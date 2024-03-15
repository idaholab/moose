[SolidProperties]
  [a]
    type = ThermalFunctionSolidProperties
    rho = 1
    cp = 1
    k = 1
  []
[]

[Components]
  [componentA]
    type = LoggerTestComponent
    log_warnings = true
    log_errors = true
  []

  [componentB]
    type = LoggerTestComponent
    log_warnings = true
    log_errors = true
  []

  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    names = '0'
    widths = '0.1'
    solid_properties = 'a'
    solid_properties_T_ref = '300'
    n_elems = 1
    n_part_elems = 1
    initial_T = 300
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

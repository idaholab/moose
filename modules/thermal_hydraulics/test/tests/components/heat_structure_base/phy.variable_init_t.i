# Tests that a function can be used to initialize temperature in a heat structure.

[GlobalParams]
[]

[Functions]
  [fn-initial_T]
    type = ParsedFunction
    expression = 'baseT + (dT * sin((pi * x) / length))'
    symbol_names = 'baseT   dT    length'
    symbol_values = '560.0  30.0  3.6576'
  []
[]

[HeatStructureMaterials]
  [fuel-mat]
    type = SolidMaterialProperties
    k = 3.65
    cp = 288.734
    rho = 1.0412e2
  []
  [gap-mat]
    type = SolidMaterialProperties
    k = 0.1
    cp = 1.0
    rho = 1.0
  []
  [clad-mat]
    type = SolidMaterialProperties
    k = 16.48672
    cp = 321.384
    rho = 6.6e1
  []
[]

[Components]
  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'

    length = 3.6576
    n_elems = 100
    names = 'FUEL GAP CLAD'
    widths = '0.0046955  0.0000955  0.000673'
    n_part_elems = '10 3 3'
    materials = 'fuel-mat gap-mat clad-mat'

    initial_T = fn-initial_T
  []

  [temp_outside]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:outer
    T = 580.0
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 0.01
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-6
  nl_max_its = 8

  l_tol = 1e-4
  l_max_its = 10
[]


[Outputs]
  [out]
    type = Exodus
  []
  [console]
    type = Console
    execute_scalars_on = none
  []
[]

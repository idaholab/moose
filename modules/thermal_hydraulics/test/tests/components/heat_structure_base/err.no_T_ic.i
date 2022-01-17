# Tests that error is generated when no initial temperature function is provided
# when not restarting.

[GlobalParams]
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
    k = 1.084498
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
  [reactor]
    type = TotalPower
    power = 296153.84615384615385
  []

  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'

    length = 1
    n_elems = 1
    names = 'FUEL GAP CLAD'
    widths = '0.0046955  0.0000955  0.000673'
    n_part_elems = '1 1 1'
    materials = 'fuel-mat gap-mat clad-mat'
  []

  [temp_outside]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:outer
    T = 600
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

  dt = 0.1
  dtmin = 1e-1

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 30

  l_tol = 1e-4
  l_max_its = 300

  start_time = 0.0
  end_time = 2.0
[]

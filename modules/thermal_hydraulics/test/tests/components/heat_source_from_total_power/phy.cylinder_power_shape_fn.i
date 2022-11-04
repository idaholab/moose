[GlobalParams]
  scaling_factor_temperature = 1e0
[]

[Functions]
  [psf]
    type = ParsedFunction
    expression = 1
  []
[]

[HeatStructureMaterials]
  [fuel-mat]
    type = SolidMaterialProperties
    k = 16
    cp = 191.67
    rho = 1.4583e4
  []
  [gap-mat]
    type = SolidMaterialProperties
    k = 64
    cp = 1272
    rho = 865
  []
  [clad-mat]
    type = SolidMaterialProperties
    k = 26
    cp = 638
    rho = 7.646e3
  []
[]

[Components]
  [reactor]
    type = TotalPower
    power = 3.0e4
  []

  [CH1:solid]
    type = HeatStructureCylindrical
    position = '0 -0.024 0'
    orientation = '0 0 1'
    length = 0.8
    n_elems = 16

    initial_T = 628.15

    names = 'fuel gap clad'
    widths = '0.003015 0.000465  0.00052'
    n_part_elems = '20 2 2'
    materials = 'fuel-mat gap-mat clad-mat'
  []

  [CH1:hgen]
    type = HeatSourceFromTotalPower
    hs = CH1:solid
    regions = 'fuel'
    power = reactor
    power_shape_function = psf
    power_fraction = 1
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
  dt = 1e-3
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-7
  nl_max_its = 40

  l_tol = 1e-5
  l_max_its = 50
[]

[Outputs]
  [out]
    type = Exodus
  []
[]

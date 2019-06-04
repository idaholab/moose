# This makes sure that we error out when HeatGeneration component is used on
# a non-existent block of a heat structure

[GlobalParams]
  scaling_factor_temperature = '1'
[]

[HeatStructureMaterials]
  [./fuel-mat]
    type = SolidMaterialProperties
    k = 2.5
    Cp = 300.
    rho = 1.032e4
  [../]
[]

[Components]
  [./reactor]
    type = PrescribedReactorPower
    power = 10
  [../]

  [./hs]
    type = HeatStructureCylindrical
    position = '0 -0.024748 0'
    orientation = '0 0 1'
    length = 3.865
    n_elems = 1

    names = 'fuel'
    widths = '0.004096'
    n_part_elems = '1'
    materials = 'fuel-mat'

    initial_T = 559.15
  [../]

  [./hgen]
    type = HeatGeneration
    power_fraction = 1
  [../]
[]

[Preconditioning]
  [./SMP_Newton]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.e-2
  dtmin = 1.e-2

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 1

  l_tol = 1e-3
  l_max_its = 30

  start_time = 0.0
  num_steps = 20

  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]

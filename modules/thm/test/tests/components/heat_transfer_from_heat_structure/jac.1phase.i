[GlobalParams]
  initial_p = 1.e5
  initial_vel = 2
  initial_T = 300

  scaling_factor_1phase = '1 1 1'
  scaling_factor_temperature = '1'

  closures = simple
[]

[FluidProperties]
  [./fp]
    type = IdealGasFluidProperties
  [../]
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
  [./pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'
    length = 2
    n_elems = 1

    A = 8.78882e-5
    D_h = 0.01179
    f = 0.01

    fp = fp
  [../]

  [./hs]
    type = HeatStructure
    position = '0 0 0'
    orientation = '0 0 1'
    length = 2
    n_elems = 1

    dim = 2
    hs_type = cylinder
    names = 'fuel'
    widths = '0.1'
    n_part_elems = '1'
    materials = 'fuel-mat'

    initial_T = 300
  [../]

  [./hx]
    type = HeatTransferFromHeatStructure1Phase
    hs = hs
    hs_side = top
    flow_channel = pipe
    Hw = 100
    P_hf = 0.029832559676
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
    petsc_options_iname = '-snes_test_err'
    petsc_options_value = ' 1e-11'
  [../]
[]

[Executioner]
  type = Transient

  start_time = 0
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 1

  l_tol = 1e-3
  l_max_its = 30

  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]

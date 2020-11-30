[GlobalParams]
  initial_p = 15.5e6
  initial_vel = 2
  initial_T = 560

  scaling_factor_1phase = '1 1 1'
  scaling_factor_temperature = '1'

  closures = simple
[]

[FluidProperties]
  [./fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  [../]
[]

[HeatStructureMaterials]
  [./fuel-mat]
    type = SolidMaterialProperties
    k = 2.5
    cp = 300.
    rho = 1.032e4
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'
    length = 3.865
    n_elems = 1

    A = 8.78882e-5
    D_h = 0.01179
    f = 0.01

    fp = fp
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

  [./hx]
    type = HeatTransferFromHeatStructure1Phase
    hs = hs
    hs_side = outer
    flow_channel = inlet  # wrong
    Hw = 5.33e4
    P_hf = 0.029832559676
  [../]

  [./hx2]
    type = HeatTransferFromHeatStructure1Phase
    hs = hs
    hs_side = outer
    flow_channel = asdf  # wrong
    Hw = 5.33e4
    P_hf = 0.029832559676
  [../]

  [./inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe:in'
    p0 = 15.5e6
    T0 = 560
  [../]

  [./outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 15e6
    legacy = true
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

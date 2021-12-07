[GlobalParams]
  initial_p = 1e5
  initial_vel = 0
  initial_T = 300

  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[HeatStructureMaterials]
  [fuel-mat]
    type = SolidMaterialProperties
    k = 2.5
    cp = 300.
    rho = 1.032e4
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase
    position = '0 0.1 0'
    orientation = '0 0 1'
    length = 4
    n_elems = 2

    A = 8.78882e-5
    D_h = 0.01179
    f = 0.01

    fp = fp
  []

  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '0 0 1'
    length = 4
    n_elems = 2

    names = 'fuel'
    widths = '0.1'
    n_part_elems = '1'
    materials = 'fuel-mat'

    initial_T = 300
  []

  [hx]
    type = HeatTransferFromHeatStructure1Phase
    hs = hs
    hs_side = outer
    flow_channel = pipe
    P_hf = 0.029832559676
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe:in'
    p0 = 1e5
    T0 = 300
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 1e5
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
  dt = 1.e-5
  solve_type = 'NEWTON'
  num_steps = 1
  abort_on_solve_fail = true
[]

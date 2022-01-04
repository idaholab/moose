[GlobalParams]
  initial_p = 1.e5
  initial_vel = 2
  initial_T = 300

  scaling_factor_1phase = '1 1 1'
  scaling_factor_temperature = '1'

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
    length = 2
    n_elems = 1

    A = 8.78882e-5
    D_h = 0.01179
    f = 0.01

    fp = fp
  []

  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '0 0 1'
    length = 2
    n_elems = 1

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
    Hw = 100
    P_hf = 0.029832559676
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
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'

  petsc_options_iname = '-snes_test_err'
  petsc_options_value = ' 1e-11'
[]

[GlobalParams]
  gravity_vector = '0 0 0'

  closures = simple_closures
[]

[FluidProperties]
  [eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[HeatStructureMaterials]
  [mat1]
    type = SolidMaterialProperties
    k = 16
    cp = 356.
    rho = 6.551400E+03
  []
[]

[Functions]
  [Ts_init]
    type = ParsedFunction
    expression = '2*sin(x*pi)+507'
  []
[]

[Components]
  [pipe1]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 5
    A = 1.907720E-04
    D_h = 1.698566E-02
    f = 0.1
  []

  [jct1]
    type = VolumeJunction1Phase
    connections = 'pipe1:out pipe2:in'
    position = '1 0 0'
    volume = 1e-5
  []

  [pipe2]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 5
    A = 1.907720E-04
    D_h = 1.698566E-02
    f = 0.1
  []

  [jct2]
    type = VolumeJunction1Phase
    connections = 'pipe2:out pipe3:in'
    position = '2 0 0'
    volume = 1e-5
  []

  [pipe3]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '2 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 5
    A = 1.907720E-04
    D_h = 1.698566E-02
    f = 0.1
  []

  [hs]
    type = HeatStructureCylindrical
    position = '1 0.01 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 5
    names = '0'
    n_part_elems = 1
    materials = 'mat1'
    widths = 0.1
    initial_T = Ts_init
  []

  [temp_outside]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:outer
    T = Ts_init
  []

  [inlet]
    type = InletVelocityTemperature1Phase
    input = 'pipe1:in'
    T = 507
    vel = 1
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe3:out'
    p = 6e6
  []

  [hx3ext]
    type = HeatTransferFromExternalAppTemperature1Phase
    flow_channel = pipe3
    P_hf = 0.0449254
    Hw = 100000
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

  dt = 0.01
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = 'newton'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100

  automatic_scaling = true

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
[]

[Outputs]
  exodus = true
  velocity_as_vector = false
[]

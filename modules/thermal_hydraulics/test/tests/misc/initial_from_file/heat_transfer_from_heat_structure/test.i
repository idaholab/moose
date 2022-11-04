# Test that the initial conditions read from the exodus file are correct

[GlobalParams]
  scaling_factor_1phase = '1. 1.e-2 1.e-4'
  scaling_factor_temperature = 1e-2

  closures = simple_closures

  initial_from_file = 'steady_state_out.e'
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
    k = 0.5
    mu = 281.8e-6
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
  [Ts_bc]
    type = ParsedFunction
    expression = '2*sin(x*pi)+507'
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase
    fp = fp
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 3
    A = 1.907720E-04
    D_h = 1.698566E-02
    f = 0.1
  []

  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 3
    names = 'wall'
    n_part_elems = 1
    materials = 'mat1'
    inner_radius = 0.01
    offset_mesh_by_inner_radius = true
    widths = 0.1
  []

  [ht]
    type = HeatTransferFromHeatStructure1Phase
    flow_channel = pipe
    hs = hs
    hs_side = INNER
    Hw = 10000
  []

  [temp_outside]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:outer
    T = Ts_bc
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe:in'
    m_dot = 0.1
    T = 500
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 6e6
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
  scheme = 'bdf2'

  start_time = 0
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-7
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
[]

[Outputs]
  exodus = true
  execute_on = 'initial'
  velocity_as_vector = false
[]

# This is a part of phy.q_wall_transfer_3eqn test. See the master file for details.

[GlobalParams]
  initial_p = 1.e5
  initial_vel = 0.
  initial_T = 300.

  closures = simple_closures
[]

[FluidProperties]
  [eos]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10

    A   = 9.6858407346e-01
    D_h = 6.1661977237e+00
    f = 0.01

    fp = eos
  []

  [hxconn]
    type = HeatTransferFromExternalAppHeatFlux1Phase
    flow_channel = pipe1
    Hw = 1e3
  []

  [inlet]
    type = SolidWall1Phase
    input = 'pipe1:in'
  []

  [outlet]
    type = SolidWall1Phase
    input = 'pipe1:out'
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
  dt = 0.5
  dtmin = 1e-7
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 20

  l_tol = 1e-3
  l_max_its = 300

  start_time = 0.0
  end_time = 5

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  exodus = true
  show = 'q_wall'
[]

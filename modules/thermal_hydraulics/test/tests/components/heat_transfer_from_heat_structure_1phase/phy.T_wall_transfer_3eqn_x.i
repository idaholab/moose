# Testing that T_solid gets properly projected onto a pipe
# That's why Hw in pipe1 is set to 0, so we do not have any heat exchange
# Note that the pipe and the heat structure have an opposite orientation, which
# is crucial for this test.

[GlobalParams]
  initial_p = 1.e5
  initial_vel = 0.
  initial_T = 300.

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
  [wall-mat]
    type = SolidMaterialProperties
    k = 100.0
    rho = 100.0
    cp = 100.0
  []
[]

[Functions]
  [T_init]
    type = ParsedFunction
    expression = '290 + sin((1 - x) * pi * 1.4)'
  []
[]

[Components]
  [pipe1]
    type = FlowChannel1Phase
    position = '0 -0.2 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 50

    A   = 9.6858407346e-01
    D_h = 6.1661977237e+00

    f = 0.01

    fp = eos
  []

  [hs]
    type = HeatStructureCylindrical
    position = '1 -0.1 0'
    orientation = '-1 0 0'
    length = 1
    n_elems = 50
    #rotation = 90

    materials = 'wall-mat'
    n_part_elems = 3
    widths = '0.1'
    names = 'wall'

    initial_T = T_init
  []

  [hxconn]
    type = HeatTransferFromHeatStructure1Phase
    hs = hs
    hs_side = outer
    flow_channel = pipe1
    Hw = 0
    P_hf = 6.2831853072e-01
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
  dt = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 20

  l_tol = 1e-5
  l_max_its = 300

  start_time = 0.0
  num_steps = 1
[]

[Outputs]
  [out]
    type = Exodus
    show = 'T_wall T_solid'
  []
  print_linear_residuals = false
[]

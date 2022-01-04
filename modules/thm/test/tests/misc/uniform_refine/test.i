[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 1e5
  initial_T = 300
  initial_vel = 0

  closures = simple_closures

  rdg_slope_reconstruction = FULL
  f = 0
  fp = eos
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
    rho = 10
    cp = 1
    k = 1
  []
[]

[Components]
  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 2
    A = 1
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 3
    A = 1
  []

  [junction]
    type = VolumeJunction1Phase
    connections = 'pipe1:out pipe2:in'
    volume = 1e-5
    position = '1 0 0'

    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 0
  []

  [inlet]
    type = SolidWall1Phase
    input = 'pipe1:in'
  []

  [outlet]
    type = SolidWall1Phase
    input = 'pipe2:out'
  []

  [hs]
    type = HeatStructureCylindrical
    position = '0 1 0'
    orientation = '1 0 0'
    length = '1'
    n_elems = '2'
    names = '0'
    widths = 0.5
    n_part_elems = '1'
    materials = 'mat1'
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
  dt = 1e-4
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-7
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10

  automatic_scaling = true
[]

[Outputs]
  exodus = true
  show = 'A'
[]

[Debug]
  show_actions = true
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [wall_temp_closures]
    type = WallTemperature1PhaseClosures
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase
    gravity_vector = '0 0 0'

    position = '0 0 0'
    orientation = '1 0 0'
    A = 1e-4
    length = 1
    n_elems = 10

    initial_vel = 0
    initial_p = 1e5
    initial_T = 300

    fp = fp
    closures = wall_temp_closures
  []

  [inlet]
    type = SolidWall1Phase
    input = 'pipe:in'
  []

  [outlet]
    type = SolidWall1Phase
    input = 'pipe:out'
  []

  [ht]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe
    T_wall = 500
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [T_wall]
    type = ADElementAverageMaterialProperty
    mat_prop = T_wall
    execute_on = 'INITIAL'
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  num_steps = 0
  dt = 1e-6

  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 5
  l_tol = 1e-3
  l_max_its = 10
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]

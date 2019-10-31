[GlobalParams]
  initial_T_liquid = 300
  initial_p_liquid = 1e5
  initial_vel_liquid = 0
  initial_T_vapor = 500
  initial_p_vapor = 1e5
  initial_vel_vapor = 0
  initial_alpha_vapor = 0.5

  f_interface = 0

  phase_interaction = true
  pressure_relaxation = true
  velocity_relaxation = true
  interface_transfer = false
  wall_mass_transfer = true

  closures = simple
[]

[FluidProperties]
  [./eos]
    type = StiffenedGas7EqnFluidProperties
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel2Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1
    A = 1
    D_h = 1
    f = 0
    Hw_liquid = 0.0
    Hw_vapor = 0.0
    fp = eos
  [../]

  [./wall_left]
    type = SolidWall
    input = 'pipe:in'
  [../]

  [./outlet]
    type = SolidWall
    input = 'pipe:out'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0.0
  dt = 0.1
  dtmin = 0.1
  num_steps = 1

  nl_max_its = 1
  l_max_its = 1
[]

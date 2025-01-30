# Tests FunctorClosures and the ability to provide multiple closures objects
# to a flow channel.
#
# Air in a sealed tube has two convective heat transfers applied, with equal
# and opposite initial temperature differences, so with equal heated perimeters
# and heat transfer coefficients, the temperature in the channel should not
# change; however, the first heat transfer, which has a higher temperature,
# has a larger heat transfer coefficient provided by its closures, so the
# temperature in the channel should increase.

[FluidProperties]
  [fp_air]
    type = AirSBTLFluidProperties
  []
[]

[Closures]
  # Note that these could be combined into a single object, but they are kept
  # separate for testing multiple closures objects:
  [friction_closures]
    type = FunctorClosures
    properties = 'f_D'
    functors = '0'
  []
  [ht_closures]
    type = FunctorClosures
    properties = 'Hw:1 Hw:2'
    functors = '10.0 1.0'
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 10.0
    n_elems = 50
    A = 0.2

    initial_p = 1e5
    initial_T = 300
    initial_vel = 0

    fp = fp_air
    closures = 'friction_closures ht_closures'
  []

  [left_wall]
    type = SolidWall1Phase
    input = 'pipe:in'
  []

  [right_wall]
    type = SolidWall1Phase
    input = 'pipe:out'
  []

  [ht1]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe
    T_wall = 400
  []
  [ht2]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe
    T_wall = 200
  []
[]

[Postprocessors]
  [T]
    type = ElementAverageValue
    variable = T
    execute_on = 'INITIAL TIMESTEP_END'
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
  scheme = bdf2
  dt = 0.1
  num_steps = 5

  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10
[]

[Outputs]
  csv = true
[]

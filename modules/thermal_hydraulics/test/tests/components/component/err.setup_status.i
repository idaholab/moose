# This test tests the setup-status-checking capability of Component. In this
# test, a Pipe component is coupled to a test component, which tries to call
# a Pipe function that retrieves data that has not been set yet. This function
# has the call that is being tested, which should produce an error because it
# is being called before Pipe's init() function was called, due to the test
# component being added before the Pipe.

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [a_test_component]
    type = TestSetupStatusComponent
    flow_channel = pipe
  []

  [pipe]
    type = FlowChannel1Phase
    fp = fp
    closures = simple_closures

    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1
    A = 1

    initial_T = 300
    initial_p = 1e5
    initial_vel = 0

    f = 0
  []

  [left_boundary]
    type = FreeBoundary
    input = 'pipe:in'
  []

  [right_boundary]
    type = FreeBoundary
    input = 'pipe:out'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

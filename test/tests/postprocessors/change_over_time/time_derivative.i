# This input file is used to test that the 'divide_by_dt' feature of ChangeOverTimePostprocessor works.

time_values     = '0 1 3 7'
function_values = '2 5 6 3'

# Expected df/dt values:
# t=0: 2-2=0
# t=1: (5-2)/1=3
# t=2: (6-5)/2=0.5
# t=3: (3-6)/4=-0.75

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [test_fn]
    type = PiecewiseLinear
    x = ${time_values}
    y = ${function_values}
  []
[]

[Postprocessors]
  [f_pp]
    type = FunctionValuePostprocessor
    function = test_fn
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [dfdt_pp]
    type = ChangeOverTimePostprocessor
    postprocessor = f_pp
    divide_by_dt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  solve = false
[]

[Times]
  [sim_times]
    type = InputTimes
    times = ${time_values}
    auto_broadcast = true
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = TimeSequenceStepper
    time_sequence = sim_times
  []
  num_steps = 3
[]

[Outputs]
  file_base = 'time_derivative'
  csv = true
[]

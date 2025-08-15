[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Times]
  [external_input]
    type = ControllableInputTimes
    nexttime = 0.01
    execute_on = 'initial timestep_begin'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 5
 [TimeSteppers]
    [external_time]
      type = TimeSequenceFromTimes
      times = external_input
    []

    [ConstDT1]
      type = ConstantDT
      dt = 10
    []

 []

[]

[Functions]
  [fake_external_time]
    type = ParsedFunction
    expression = '2*t + 0.5'
  []
[]

[Controls]
  [func_control]
    type = RealFunctionControl
    parameter = 'Times/external_input/nexttime'
    function = 'fake_external_time'
    execute_on = 'initial timestep_begin'
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]

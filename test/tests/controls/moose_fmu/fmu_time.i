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
    next_time = 0.1
    execute_on = 'INITIAL MULTIAPP_FIXED_POINT_BEGIN MULTIAPP_FIXED_POINT_END'
  []
[]

[Problem]
  solve = false
[]

[Functions]
  [dts]
    type = ParsedFunction
    expression = 0.1*exp(t*0.5)
  []
[]


[Executioner]
  type = Transient
  num_steps = 10
 [TimeSteppers]
    [external_time]
      type = TimeSequenceFromTimes
      times = external_input
    []

    [ConstDT1]
      type = ConstantDT
      dt = 0.15
    []
 []

[]

[Controls]
  [web_server]
    type = WebServerControl
    execute_on = 'INITIAL  MULTIAPP_FIXED_POINT_BEGIN MULTIAPP_FIXED_POINT_END'
  []
[]


[Outputs]
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]

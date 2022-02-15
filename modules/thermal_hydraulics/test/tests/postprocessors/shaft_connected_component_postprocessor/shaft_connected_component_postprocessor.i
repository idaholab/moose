# Tests ShaftConnectedComponentPostprocessor

[Components]
  [motor]
    type = ShaftConnectedMotor
    inertia = 1
    torque = 2
  []
  [shaft]
    type = Shaft
    connected_components = 'motor'
    initial_speed = 0
  []
[]

[Postprocessors]
  [motor_inertia]
    type = ShaftConnectedComponentPostprocessor
    shaft_connected_component_uo = motor:shaftconnected_uo
    quantity = inertia
    execute_on = 'INITIAL'
  []
  [motor_torque]
    type = ShaftConnectedComponentPostprocessor
    shaft_connected_component_uo = motor:shaftconnected_uo
    quantity = torque
    execute_on = 'INITIAL'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]

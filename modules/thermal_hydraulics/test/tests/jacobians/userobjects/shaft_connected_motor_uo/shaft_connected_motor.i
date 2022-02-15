[JacobianTestGeneral]
  scalar_variable_names = 'shaft_omega'
  scalar_variable_values = '1'
  use_transient_executioner = true
[]

[UserObjects]
  [motor_uo]
    type = ShaftConnectedMotorUserObject
    torque = 12
    inertia = 23
    shaft_speed = shaft_omega
  []
[]

[ScalarKernels]
  [shaft_td]
    type = ShaftTimeDerivativeScalarKernel
    variable = shaft_omega
    uo_names = motor_uo
  []

  [shaft_total_torque]
    type = ShaftComponentTorqueScalarKernel
    variable = shaft_omega
    shaft_connected_component_uo = motor_uo
  []
[]

!include 2d-velocity-pressure.i

[Variables]
  [extra_pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0.0
  []
[]

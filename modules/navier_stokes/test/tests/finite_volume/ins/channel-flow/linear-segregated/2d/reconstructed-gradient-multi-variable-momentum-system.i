!include 2d-velocity-pressure.i

[Variables]
  [extra_u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = 0.0
  []
[]

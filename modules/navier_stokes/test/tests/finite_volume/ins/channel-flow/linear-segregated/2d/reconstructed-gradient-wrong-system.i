!include 2d-velocity-pressure.i

[Variables]
  [wrong_pressure]
    type = MooseLinearVariableFVReal
    solver_sys = wrong_pressure_system
    initial_condition = 0.2
    gradient_method = reconstructed
  []
[]

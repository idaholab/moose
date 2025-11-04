!include base.i

[Convergence]
  [steady_conv]
    type = DefaultSteadyStateConvergence
    steady_state_tolerance = ${ss_tol}
  []
[]

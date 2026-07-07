# Solves problem in a single nonlinear system.

!include base.i

[Convergence]
  [nl_conv]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-8
    nl_rel_tol = 1e-8
    nl_max_its = 10
  []
[]

[Executioner]
  nonlinear_convergence = nl_conv
[]

[Outputs]
  file_base = single_system
[]

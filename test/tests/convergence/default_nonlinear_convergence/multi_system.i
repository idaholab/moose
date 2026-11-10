!include ../../multisystem/picard/nonlinearfe_nonlinearfe/multi_system.i

[Convergence]
  [u_conv]
    nl_abs_tol := 0
    nl_rel_tol := 1e-8
    verbose = true
  []
  [v_conv]
    nl_abs_tol := 1e-9
    nl_rel_tol := 0
    verbose = true
  []
  [fp_conv_1iteration]
    type = IterationCountConvergence
    max_iterations = 1
    converge_at_max_iterations = true
  []
[]

[Executioner]
  multi_system_fixed_point_convergence := fp_conv_1iteration
[]

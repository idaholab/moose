# Solves problem in a multi-system fixed point solve.

!include base.i

[Problem]
  nl_sys_names = 'u_sys v_sys'
[]

[Variables]
  [u]
    solver_sys = u_sys
  []
  [v]
    solver_sys = v_sys
  []
[]

[Postprocessors]
  [res_norm]
    type = Residual
    residual_type = COMPUTE
    execute_on = 'MULTISYSTEM_FIXED_POINT_CONVERGENCE'
  []
[]

[Convergence]
  [u_conv]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-8
    nl_rel_tol = 1e-8
    nl_max_its = 10
  []
  [v_conv]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-8
    nl_rel_tol = 1e-8
    nl_max_its = 10
  []
  [fp_conv]
    type = PostprocessorConvergence
    postprocessor = res_norm
    tolerance = 1e-8
    max_iterations = 12
    verbose = true
  []
[]

[Executioner]
  nonlinear_convergence = 'u_conv v_conv'
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = fp_conv
[]

[Outputs]
  file_base = multi_system
[]

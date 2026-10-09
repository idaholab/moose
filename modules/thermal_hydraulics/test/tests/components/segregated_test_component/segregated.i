# Solves two coupled Poisson equations in separate nonlinear systems using a
# segregated (multi-system fixed-point) solve driven by a THM component:
#
#   -div(grad(comp_u)) = 0,       comp_u(in) = 0, comp_u(out) = 1
#   -div(grad(comp_v)) = comp_u,  comp_v(in) = 0, comp_v(out) = 0
#
# 'comp_u' is added to solver system 'nl0' and 'comp_v' to 'v_sys' by
# SegregatedTestComponent, exercising the solver-system routing in
# Simulation::addSimVariable.

[Problem]
  nl_sys_names = 'nl0 v_sys'
[]

[Components]
  [comp]
    type = SegregatedTestComponent
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 10
    u_solver_system = nl0
    v_solver_system = v_sys
  []
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = comp_u
    execute_on = 'TIMESTEP_END'
  []
  [v_avg]
    type = ElementAverageValue
    variable = comp_v
    execute_on = 'TIMESTEP_END'
  []
  [res_norm]
    type = Residual
    residual_type = COMPUTE
    execute_on = 'MULTISYSTEM_FIXED_POINT_ITERATION_END'
    outputs = none
  []
[]

[Convergence]
  [u_conv]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
    nl_rel_tol = 1e-8
    nl_max_its = 20
  []
  [v_conv]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
    nl_rel_tol = 1e-8
    nl_max_its = 20
  []
  [fp_conv]
    type = PostprocessorConvergence
    postprocessor = res_norm
    tolerance = 1e-8
    max_iterations = 20
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nonlinear_convergence = 'u_conv v_conv'
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = fp_conv
  l_tol = 1e-8
  l_max_its = 100
[]

[Outputs]
  csv = true
[]

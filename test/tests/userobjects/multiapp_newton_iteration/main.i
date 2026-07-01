# Main-app for MultiAppNewtonIterationUserObject test.
#
# Sub-app ODE:  du/dt = param_pp,  u(0) = 0
#
# Target function: u_target(t) = t^2
#
# The Newton iteration finds param_pp*(t) such that
#   u_prev + param_pp * dt = t^2
# => param_pp*(t) = (t^2 - (t - dt)^2) / dt = 2*t - dt
#
# With dt = 1: param_pp*(1) = 1, param_pp*(2) = 3, param_pp*(3) = 5.
# Because the sub-app model is linear in param_pp, Newton converges in at
# most two iterations per time step.
#
# Verification: sub-app output_pp (= u) must equal 1, 4, 9 at t = 1, 2, 3.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  # Placeholder scalar variable so the main-app nonlinear system is non-empty.
  [dummy]
    family = SCALAR
    order = FIRST
    initial_condition = 0.0
  []
[]

[ScalarKernels]
  # Zero-residual kernel; the main app itself has nothing to solve.
  [null]
    type = NullScalarKernel
    variable = dummy
  []
[]

[Functions]
  # Target: u_target(t) = t^2
  [target_fn]
    type = ParsedFunction
    expression = 't^2'
  []
[]

[Postprocessors]
  # Receives the converged parameter value from the Newton UserObject each time step.
  [param_value]
    type = Receiver
    default = 0.0
  []
[]

[UserObjects]
  [newton_uo]
    type = MultiAppNewtonIterationUserObject
    sub_app_input = sub.i
    param_postprocessor = param_pp          # Receiver PP in sub.i
    output_postprocessor = output_pp        # ScalarVariable PP reading u in sub.i
    target_function = target_fn
    initial_parameter = 1.0                 # initial guess
    delta_parameter = 1e-3
    abs_tol = 1e-10
    rel_tol = 1e-8
    max_iterations = 50
    parameter_postprocessor = param_value   # Publish converged p to main-app PP
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
  solve_type = NEWTON
[]

[Outputs]
  # main_out.csv records the converged parameter (param_value) each time step.
  # The sub-app output_pp (= u) reaches 1, 4, 9 at t = 1, 2, 3 (sub.i has csv = false).
  csv = true
[]

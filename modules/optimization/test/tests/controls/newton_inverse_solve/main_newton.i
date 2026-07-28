# Driver for NewtonInversionControl on a nonlinear forward model.
#
# Forward model: du/dt = p^3 (u accumulates p^3 each unit step). Target f(t) = 1000*t
# needs du = 1000 per step  =>  p^3 = 1000  =>  root p = 10 at every step (u = 1000*t).
# Initial guess p0 = -3 (wrong side of the inflection at p = 0). Finite-difference
# Newton samples a fresh local derivative each step and converges to p = 10; the
# secant method (see main_secant.i) enters a chaotic orbit and never converges.
#
# NewtonInversionControl uses two fixed-point iterations per Newton step (base solve
# at p, perturbed solve at p + parameter_delta), so allow a larger max_iterations.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [dummy]
    family = SCALAR
    order = FIRST
    initial_condition = 0.0
  []
[]

[ScalarKernels]
  [null]
    type = NullScalarKernel
    variable = dummy
  []
[]

[Functions]
  [target_fn]
    type = ParsedFunction
    expression = '1000*t'
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = sub.i
    execute_on = TIMESTEP_BEGIN
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppPostprocessorTransfer
    to_multi_app = sub
    from_postprocessor = p
    to_postprocessor = param_pp
  []
  [from_sub]
    type = MultiAppPostprocessorTransfer
    from_multi_app = sub
    from_postprocessor = output_pp
    to_postprocessor = output
    reduction_type = average
  []
[]

[Postprocessors]
  # Working parameter guess: transferred to the sub, read and updated by the Control.
  [p]
    type = Receiver
    default = -3.0
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  # Sub-app output, filled by the FROM transfer.
  [output]
    type = Receiver
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  # Convergence residual written by the Control (|y_base - target| on base iterations,
  # a large sentinel on perturbed iterations).
  [residual]
    type = Receiver
    default = 1e30
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  # Published converged parameter (the inverse-problem solution). This is the CSV output.
  [param_value]
    type = Receiver
    default = 0.0
  []
[]

[Controls]
  [newton]
    type = NewtonInversionControl
    output_postprocessor = output
    parameter_postprocessor = p
    residual_postprocessor = residual
    converged_parameter_postprocessor = param_value
    target_function = target_fn
    parameter_delta = 1e-3
  []
[]

[Convergence]
  [inv_conv]
    type = PostprocessorConvergence
    postprocessor = residual
    tolerance = 1.0
    max_iterations = 25
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
  solve_type = NEWTON
  multiapp_fixed_point_convergence = inv_conv
[]

[Outputs]
  csv = true
[]

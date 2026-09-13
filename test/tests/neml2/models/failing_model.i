# A deliberately non-converging NEML2 model used to exercise
# dump_inputs_on_failure. It is a minimal scalar backward-Euler update whose
# Newton solver is capped at zero iterations (max_its = 0), so against the
# non-zero initial residual produced by the external x_rate forcing the solve
# always raises the recoverable ConvergenceError without taking a step. This
# keeps the failure deterministic and model-agnostic -- no stiffness tuning.

[Models]
  [integrate]
    type = ScalarBackwardEulerTimeIntegration
    variable = 'x'
    time = 't'
  []
[]

[EquationSystems]
  [eq_sys]
    type = NonlinearSystem
    model = 'integrate'
    unknowns = 'x'
    residuals = 'x_residual'
  []
[]

[Solvers]
  [newton]
    type = Newton
    abs_tol = 1e-10
    rel_tol = 1e-08
    max_its = 0
    linear_solver = 'lu'
  []
  [lu]
    type = DenseLU
  []
[]

[Models]
  [model]
    type = ImplicitUpdate
    equation_system = 'eq_sys'
    solver = 'newton'
  []
[]

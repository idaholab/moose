# TangentPredictor

!syntax description /Executioner/Predictor/TangentPredictor

The `TangentPredictor` computes a solution increment from the tangent stiffness at the last
accepted solution and the external load increment that produced that accepted step. It is intended
for quasistatic problems with time-ramped residual load terms.

!alert warning
The auxiliary residual and Jacobian assemblies used by this predictor temporarily evaluate tagged
load residuals at the previous and accepted times. Use this predictor only when the objects
contributing to [!param](/Executioner/Predictor/TangentPredictor/load_vector_tag) depend on the
current time `t`, the current solution, and ordinary material state. The predictor is not safe with
stateful `EXEC_LINEAR`-triggered subsystems, including transfers, multi-apps, time-dependent
Controls, UserObjects with `EXEC_LINEAR`, and AuxKernels. It is also not safe with stateful
`EXEC_LINEAR`/`EXEC_NONLINEAR` postprocessor side effects.

!alert warning
Tagged load functions used with [!param](/Executioner/Predictor/TangentPredictor/load_vector_tag)
should depend on `t` alone. During the predictor's auxiliary residual evaluations, `dt`, `t_step`,
and `timeOld` are not shifted consistently with the temporary time value.

The accepted-step tangent direction $d_n$ is computed from the distributed linear solve:

!equation
K_T(u_n) d_n = \Delta F_n

where $K_T(u_n)$ is the tangent matrix assembled at the accepted solution $u_n$ and
$\Delta F_n$ is assembled from the residual vector tag supplied with the
[!param](/Executioner/Predictor/TangentPredictor/load_vector_tag) parameter:

!equation
\Delta F_n = R_\text{load}(u_n, t_{n-1}) - R_\text{load}(u_n, t_n)

The next initial guess is then:

!equation
u_\text{pred} = u_n + s \frac{\Delta t_{n+1}}{\Delta t_n} d_n

with $s$ given by [!param](/Executioner/Predictor/TangentPredictor/scale).

The predictor does not form or store $K_T^{-1}$. It assembles the tangent matrix and uses MOOSE's
distributed libMesh/PETSc linear solver infrastructure to solve for $d_n$ by default.
The predictor linear solve uses the executioner's linear tolerance and maximum iteration settings
unless [!param](/Executioner/Predictor/TangentPredictor/linear_solve_tol) or
[!param](/Executioner/Predictor/TangentPredictor/linear_solve_max_its) are supplied. These
predictor-specific settings are often useful because the solve only produces an initial guess. The
[!param](/Executioner/Predictor/TangentPredictor/linear_solver_options_prefix) parameter can be used
for prefixed PETSc KSP/PC options.

For a cheaper approximation, set
[!param](/Executioner/Predictor/TangentPredictor/use_diagonal_approximation)=`true`. In that mode
the predictor still assembles the full tangent matrix, extracts its diagonal, and computes the
Jacobi approximation

!equation
d_n \approx \operatorname{diag}\left(K_T(u_n)\right)^{-1} \Delta F_n

with distributed vector operations. This is less accurate than the full tangent solve. It can be
cheaper when the auxiliary KSP solve is expensive, but it does not avoid tangent assembly: the
diagonal includes the same kernels, boundary conditions, and constraints as the full solve. Only the
auxiliary KSP solve is skipped.

## Tagged load terms

Only residual objects that contribute to the supplied load vector tag are assembled to form
$\Delta F_n$. The tag must be declared as an extra residual vector tag on the problem, and load
objects must opt in with `extra_vector_tags`.

!listing test/tests/predictors/tangent_predictor/tangent_predictor_test.i block=Problem

!listing test/tests/predictors/tangent_predictor/tangent_predictor_test.i block=Kernels/load

This opt-in tagging keeps internal stiffness terms separate from external load increments. The
predictor also applies nodal boundary-condition residuals to the tagged load increment so constrained
rows are compatible with the assembled tangent. The predictor is exact for linear quasistatic
problems with tagged linearly ramped loads that depend only on the current time `t` and a constant
tangent. It is only an approximation when stiffness, material state, contact active sets, or other
nonlinear effects change between accepted steps.

## Example input syntax

!listing test/tests/predictors/tangent_predictor/tangent_predictor_test.i block=Executioner

!syntax parameters /Executioner/Predictor/TangentPredictor

!syntax inputs /Executioner/Predictor/TangentPredictor

!syntax children /Executioner/Predictor/TangentPredictor

# EigenvectorBNorm

!syntax description /Postprocessors/EigenvectorBNorm

## Description

For the generalized eigenproblem $A\phi = \lambda B\phi$ that an [Eigenvalue.md] executioner hands to
SLEPc, this post-processor reports the $B$-norm of the current eigenvector,

\begin{equation}
c(\phi) = \sqrt{\phi^{\top} B \phi},
\end{equation}

where $\phi$ is the eigenvector in the solution of the eigen system and $B$ is the operator that the
eigen solve assembled. This object does not compute the quadratic form itself. The
[EigenProblem.md] computes $\phi^{\top} B \phi$ and stores it, and this object reports the square root
of the stored value. The value is zero before the first solve.

The eigen problem computes the quadratic form only when an input needs it: when an `EigenvectorBNorm`
exists, or when [!param](/Executioner/Eigenvalue/eigenvector_normalization) is `b_norm`. It computes
the value right after each solve, before the [!param](/Executioner/Eigenvalue/normalization) hook
scales the active eigenvector, and each time the executioner loads an eigenvector for output. When
the eigenvector is scaled by a factor $\alpha$, the stored value is multiplied by $\alpha^2$, so the
reported value stays current without a new matrix-vector product.

The quadratic form accounts for the sign that
[!param](/Problem/EigenProblem/negative_sign_eigen_kernel) applies to the eigen-tagged kernels, also
with `PJFNKMO`, whose assembled $B$ does not carry that sign. For the conventional input, a
[CoefReaction.md] with `coefficient = -1` and `extra_vector_tags = 'eigen'` and the default
`negative_sign_eigen_kernel = true`, $B$ is the positive-definite mass matrix and $c(\phi)$ is the
mass norm of the mode. For a standard eigenproblem SLEPc holds no $B$ operator, $B = I$, and the value
is the Euclidean norm $\|\phi\|_2$. A condensed solve, for example with hanging-node constraints, is
evaluated on the same degrees of freedom as the condensed operator.

The square root makes the value usable as a normalization target. The
[!param](/Executioner/Eigenvalue/normalization) hook scales the solution by
[!param](/Executioner/Eigenvalue/normal_factor) divided by the post-processor value and repeats until
the two agree, so the post-processor has to be homogeneous of degree one in $\phi$. The $B$-norm
satisfies $c(\alpha\phi) = \alpha\, c(\phi)$ for a scale factor $\alpha > 0$, so the first scaling
lands exactly and the loop converges in one step. The quadratic form $\phi^{\top} B \phi$ itself is
degree two, and scaling against it oscillates instead of converging.

$B$ has to be assembled, not applied matrix-free. Use a solve type that assembles $B$: a linear
SLEPc solver such as `KRYLOVSCHUR` or `JACOBI_DAVIDSON`, or `PJFNKMO`. `NEWTON` evaluates the
products with $B$ as residuals and never assembles $B$, so the problem setup reports an error. With
a matrix-free [!param](/Executioner/Eigenvalue/solve_type) such as the default `PJFNK`, or `JFNK`,
$B$ is a shell matrix, and the eigen problem reports an error after the solve.

$c(\phi)$ is a norm only when $B$ is symmetric positive definite. The eigen kernels and
[!param](/Problem/EigenProblem/negative_sign_eigen_kernel) can together give an indefinite or
negative definite $B$. A negative $\phi^{\top} B \phi$ has no square root, and the eigen problem
reports an error.

The default [!param](/Postprocessors/EigenvectorBNorm/execute_on) is `timestep_end`, which puts the
value in the output. To use this post-processor as the
[!param](/Executioner/Eigenvalue/normalization) post-processor, add `linear` to its
[!param](/Postprocessors/EigenvectorBNorm/execute_on): the executioner requires that post-processor
to execute on `linear`, and the normalization hook reads it again after each `linear` execution.

To mass-normalize every output mode, set
[!param](/Executioner/Eigenvalue/output_all_eigenvectors) to `true` and
[!param](/Executioner/Eigenvalue/eigenvector_normalization) to `b_norm`. Each mode written then
satisfies $\phi_i^{\top} B \phi_i = 1$; see [Eigenvalue.md#output-all-eigenvectors]. That option uses the stored quadratic form directly and
does not need this post-processor. Add this post-processor to report the $B$-norm of each mode, or
to normalize the active eigenvector through the
[!param](/Executioner/Eigenvalue/normalization) hook.

## Example Input File Syntax

In this example, the five smallest modes of a diffusion-reaction eigenproblem are written with a unit
$B$-norm. `bnorm` reports the $B$-norm of each mode, and `umax` reports the peak magnitude of each
mode.

!listing test/tests/problems/eigen_problem/output_all_eigenvectors/all_modes.i block=Postprocessors

!syntax parameters /Postprocessors/EigenvectorBNorm

!syntax inputs /Postprocessors/EigenvectorBNorm

!syntax children /Postprocessors/EigenvectorBNorm

# EigenvectorBNorm

!syntax description /Postprocessors/EigenvectorBNorm

## Description

For the generalized eigenproblem $A\phi = \lambda B\phi$ that an [Eigenvalue.md] executioner hands to
SLEPc, this post-processor returns the $B$-norm of the current eigenvector,

\begin{equation}
c(\phi) = \sqrt{\phi^{\top} B \phi},
\end{equation}

where $\phi$ is the current solution of the eigen system and $B$ is the operator the eigen solver
actually solved with. $B$ is read back from the SLEPc eigensolver, so it carries the sign that
[!param](/Problem/EigenProblem/negative_sign_eigen_kernel) applies to the eigen-tagged kernels: for
the conventional input, a `CoefReaction` with `coefficient = -1` and `extra_vector_tags = 'eigen'`
and the default `negative_sign_eigen_kernel = true`, $B$ is the positive-definite mass matrix and
$c(\phi)$ is the mass norm of the mode.

The square root is what makes the value usable as a normalization target. The
[!param](/Executioner/Eigenvalue/normalization) hook scales the solution by
[!param](/Executioner/Eigenvalue/normal_factor) divided by the post-processor value and repeats until
the two agree, so the post-processor has to be homogeneous of degree one in $\phi$. The $B$-norm
satisfies $c(\alpha\phi) = \alpha\, c(\phi)$, so the first scaling lands exactly and the loop
converges in one step. The quadratic form $\phi^{\top} B \phi$ itself is degree two, and scaling against it oscillates
instead of converging.

For a standard eigenproblem SLEPc holds no $B$ operator, $B = I$, and the value is the Euclidean norm
$\|\phi\|_2$. Before the first solve, when the eigensolver has no operators yet, the value is zero.

$B$ has to be assembled, not applied matrix-free. Use a solve type that assembles $B$ (the linear
SLEPc solvers such as `KRYLOVSCHUR` or `JACOBI_DAVIDSON`, `NEWTON`, `PJFNKMO`). With a matrix-free
solve type such as the default `PJFNK`, or `JFNK`, $B$ is a shell matrix whose product would re-enter
the residual evaluation from inside a `linear` post-processor, and this object reports

```
EigenvectorBNorm requires an assembled B matrix, but this eigenvalue solve forms B matrix-free. Use a solve type that assembles B, i.e. a linear eigen solve type such as KRYLOVSCHUR or JACOBI_DAVIDSON, or NEWTON or PJFNKMO.
```

A negative $\phi^{\top} B \phi$ has no square root and is also an error; it means the eigen kernels
and [!param](/Problem/EigenProblem/negative_sign_eigen_kernel) together give an indefinite $B$.

The default [!param](/Postprocessors/EigenvectorBNorm/execute_on) is `'linear timestep_end'`. The
`linear` flag is required, because the normalization hook reads the post-processor during the scaling
loop and not at a time step boundary; `timestep_end` puts the converged value in the output.

Set `normalization` to this post-processor and `normal_factor = 1` to mass-normalize the output, so
that every mode written satisfies $\phi_i^{\top} B \phi_i = 1$. Together with
[!param](/Executioner/Eigenvalue/output_all_eigenvectors) that normalization is applied to every
converged mode; see [Eigenvalue.md#output-all-eigenvectors].

## Example Input File Syntax

In this example, the five smallest modes of a diffusion-reaction eigenproblem are each normalized to
a unit $B$-norm through `bnorm`, while `umax` reports the peak magnitude of each mode.

!listing test/tests/problems/eigen_problem/output_all_eigenvectors/all_modes.i block=Postprocessors

!syntax parameters /Postprocessors/EigenvectorBNorm

!syntax inputs /Postprocessors/EigenvectorBNorm

!syntax children /Postprocessors/EigenvectorBNorm

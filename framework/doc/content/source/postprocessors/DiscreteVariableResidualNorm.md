# DiscreteVariableResidualNorm

This postprocessor computes one of the following discrete norms of the nonlinear residual
vector $\mathbf{r}$ for a variable $u$ over a chosen domain $\Omega$:

- $\ell_1$:

  !equation
  \|r\|_1 = \sum\limits_{i\in \mathcal{I}_{u,\Omega}} |r_i|

- $\ell_2$:

  !equation
  \|r\|_2 = \sqrt{\sum\limits_{i\in \mathcal{I}_{u,\Omega}} |r_i|^2}

- $\ell_\infty$:

  !equation
  \|r\|_\infty = \max\limits_{i\in \mathcal{I}_{u,\Omega}} |r_i|

where $\mathcal{I}_{u,\Omega}$ is the set of residual vector indices corresponding to the
solution variable $u$ and the domain $\Omega$.

Note that some of these algebraic vector norms have a mesh-size bias. For example, if all vector entries
were equal: $r_i = \bar{r}, \forall i$, then

!equation
\|r\|_1 = N \bar{r}

!equation
\|r\|_2 = \sqrt{N} \bar{r}

!equation
\|r\|_\infty = \bar{r}

where $N$ is the size of the set $\mathcal{I}_{u,\Omega}$.
The parameter [!param](/Postprocessors/DiscreteVariableResidualNorm/correct_mesh_bias)
can be specified to divide the norm by the respective $N$ bias ($N$, $\sqrt{N}$, and $1$
for $\ell_1$, $\ell_2$, and $\ell_\infty$, respectively).

Note the related post-processors:

- [VariableResidual.md]: computes the $\ell_2$ norm of the nonlinear residual norm for a specific variable $u$ over the domain of $u$, $\Omega_u$.
- [/Residual.md]: computes the $\ell_2$ norm of the nonlinear residual norm for all variables over the entire domain $\Omega_\text{total}$.

!syntax parameters /Postprocessors/DiscreteVariableResidualNorm

!syntax inputs /Postprocessors/DiscreteVariableResidualNorm

!syntax children /Postprocessors/DiscreteVariableResidualNorm

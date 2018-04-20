# MatchedValueBC

!syntax description /BCs/MatchedValueBC

## Description

`MatchedValueBC` is a `NodalBC` which applies to systems of two or more variables,
and can be used to impose equality of two solutions along a given `boundary`.
This class is appropriate for systems of partial differential equations (PDEs) of
the form
\begin{equation}
\begin{aligned}
  -\nabla^2 u &= f_1 && \quad \in \Omega \\
  -\nabla^2 v &= f_2 && \quad \in \Omega \\
  \frac{\partial u}{\partial n} &= h_1 && \quad \in \partial \Omega_N \\
  \frac{\partial v}{\partial n} &= h_2 && \quad \in \partial \Omega_N \\
  u &= v && \quad \in \partial \Omega_D,
\end{aligned}
\end{equation}
where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary,
$u$, $v$ are the unknowns, $f_1$, $f_2$ are forcing functions (which
may depend on both $u$ and $v$), and $h_1$ and $h_2$ are given
fluxes. The `v` parameter is used to specify the variable whose value
is tied to $u$. In the example below, the other variable's name
happens to be `v` as well.

## Example Input Syntax

!listing test/tests/bcs/matched_value_bc/matched_value_bc_test.i start=[./left_u] end=[../] include-end=true

!syntax parameters /BCs/MatchedValueBC

!syntax inputs /BCs/MatchedValueBC

!syntax children /BCs/MatchedValueBC

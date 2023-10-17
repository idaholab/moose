# ADMatHeatSource

## Description

`ADMatHeatSource` implements a force term in thermal transport to represent a
heat source. The strong form, given adomain $\Omega$ is defined as

\begin{equation}
\underbrace{-f}_{\textrm{BodyForce}} + \sum_{i=1}^n \beta_i = 0 \in \Omega
\end{equation}
where $f$ is the source term (negative if a sink) and the second term on the
left hand side represents the strong forms of other kernels. The `BodyForce`
weak form, in inner-product notation, is defined as

\begin{equation}
R_i(u_h) = (\psi_i, -f) \quad \forall \psi_i,
\end{equation}
where the $\psi_i$ are the test functions, and $u_h$ are the trial solutions in
the finite dimensional space $\mathcal{S}^h$ for the unknown ($u$).

Here, $f$ is given as a material property with an optional constant scalar. The
Jacobian is calculated automatically via automatic differentiation.

!syntax parameters /Kernels/ADMatHeatSource

!syntax inputs /Kernels/ADMatHeatSource

!syntax children /Kernels/ADMatHeatSource

!bibtex bibliography

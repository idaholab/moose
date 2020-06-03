# ADVectorMatchedValueBC

!syntax description /BCs/ADVectorMatchedValueBC

## Description

`ADVectorMatchedValueBC` is a `ADVectorNodalBC` which applies to systems of two or more variables,
and can be used to impose equality of two solutions along a given `boundary`.
This class is appropriate for systems of partial differential equations (PDEs) of
the form
\begin{equation}
\begin{aligned}
  -\nabla^2 \vec{u} &= \vec{f}_1 && \quad \in \Omega \\
  -\nabla^2 \vec{v} &= \vec{f}_2 && \quad \in \Omega \\
  \frac{\partial \vec{u}}{\partial n} &= \vec{h}_1 && \quad \in \partial \Omega_N \\
  \frac{\partial \vec{v|}{\partial n} &= \vec{h}_2 && \quad \in \partial \Omega_N \\
  \vec{u} &= \vec{v} && \quad \in \partial \Omega_D,
\end{aligned}
\end{equation}
where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary,
$\vec{u}$, $\vec{v}$ are the unknowns, $\vec{f}_1$, $\vec{f}_2$ are forcing functions (which
may depend on both $\vec{u}$ and $\vec{v}$), and $\vec{h}_1$ and $\vec{h}_2$ are given
fluxes. The `v` parameter is used to specify the variable whose value
is tied to $\vec{u}$. In the example below, the other variable's name
happens to be `v` as well.

## Example Input Syntax

!listing test/tests/interfacekernels/ad_coupled_vector_value/coupled.i start=[./middle] end=[../] include-end=true

!syntax parameters /BCs/ADVectorMatchedValueBC

!syntax inputs /BCs/ADVectorMatchedValueBC

!syntax children /BCs/ADVectorMatchedValueBC

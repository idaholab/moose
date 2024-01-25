# INSFVScalarFieldAdvection

This object adds a $\nabla \cdot \vec u \phi$ term for an arbitrary scalar field
$\phi$, where $\phi$ corresponds to the nonlinear variable that this kernel acts
on. The nonlinear `variable` can be of type `MooseVariableFVReal` or for
consistency with other INSFV naming conventions, can be of type
[`INSFVScalarFieldVariable`](INSFVScalarFieldVariable.md).

When using a mixture model for multiphase flows, this kernel also allows us
to add the slip velocity, which is modeled by the following term:

\begin{equation}
  \nabla \cdot \bm{v}_{slip,d} \phi \,,
\end{equation}

where:

- $\bm{v}_{slip,d}$ is the slip velocity of the transported phase

When adding the slip velocity, the net advection term that is added is the following
$\bm{v}_{d} = \bm{v} + \bm{v}_{slip,d}$, where $\bm{v}$ is the mixture velocity.

!syntax parameters /FVKernels/INSFVScalarFieldAdvection

!syntax inputs /FVKernels/INSFVScalarFieldAdvection

!syntax children /FVKernels/INSFVScalarFieldAdvection

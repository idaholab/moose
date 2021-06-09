# InterfaceDiffusiveFluxIntegral

!syntax description /Postprocessors/InterfaceDiffusiveFluxIntegral

The diffusive flux integral is defined as
\begin{equation}
  \int_{\partial \Omega} -D \vec{\nabla} u \cdot \vec{n} d\Omega
\end{equation}
with $\partial \Omega$ the interface, $D$ the diffusion coefficient, $u$ the variable and
$\vec{n}$ the normal to the interface.

The integral of the diffusive flux may be used to compute the contribution of diffusion when
examining the balance of a physical quantity (energy, momentum) over a domain.

!alert note
For finite volume variables, this postprocessor computes the diffusive flux using a two
point gradient. This is only accurate if the (interface) kernel is also computing gradients
this way. Currently, only [FVDiffusionInterface](/fviks/FVDiffusionInterface.md)
is computing gradients this way.

!syntax parameters /Postprocessors/InterfaceDiffusiveFluxIntegral

!syntax inputs /Postprocessors/InterfaceDiffusiveFluxIntegral

!syntax children /Postprocessors/InterfaceDiffusiveFluxIntegral

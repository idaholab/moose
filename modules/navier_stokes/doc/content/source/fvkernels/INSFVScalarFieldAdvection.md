# INSFVScalarFieldAdvection

This object adds a $\nabla \cdot \vec u \phi$ term for an arbitrary scalar field
$\phi$, where $\phi$ corresponds to the nonlinear variable that this kernel acts
on. The nonlinear `variable` can be of type `MooseVariableFVReal` or for
consistency with other INSFV naming conventions, can be of type
[`INSFVScalarFieldVariable`](INSFVScalarFieldVariable.md).

## Advection relative to the mixture

The scalar may also be carried at a velocity of its own relative to the mixture. Supplying that
relative velocity under `u_slip`, and its other components, adds

\begin{equation}
  \nabla \cdot \bm{u}_{r} \phi \,,
\end{equation}

so that the net advecting velocity is $\bm{u} + \bm{u}_{r}$, where $\bm{u}$ is the mixture
velocity and $\bm{u}_{r}$ is the relative velocity supplied.

For the two-phase mixture model the quantity to supply is the *drift* velocity of the dispersed
phase, $\bm{u}_{Md} = \bm{u}_d - \bm{u}_m$, which is the velocity of that phase relative to the
centre of mass of the mixture and is what its transport equation is advected at. It is not the
slip velocity $\bm{u}_{slip,d} = \bm{u}_d - \bm{u}_c$, which is measured against the continuous
phase; the two differ by the dispersed phase mass fraction and agree only in the dilute limit.
[WCNSFV2PSlipVelocityFunctorMaterial.md] declares both, the drift velocity under
`drift_velocity_name`. The parameter name here is historical.

The two halves of the flux are interpolated separately, each upwinded on the sign of its own
velocity, so a face whose flux is carried by the relative velocity takes its value from the right
side even where the mixture velocity is small or opposed to it. Both halves use the limiter of
`advected_interp_method`.

!syntax parameters /FVKernels/INSFVScalarFieldAdvection

!syntax inputs /FVKernels/INSFVScalarFieldAdvection

!syntax children /FVKernels/INSFVScalarFieldAdvection

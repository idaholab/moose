# NSFVPressureWorkFunctorMaterial

This material computes the pressure work density carried by an advecting velocity, the material
derivative of the pressure,

\begin{equation}
  \frac{Dp}{Dt} = \frac{\partial p}{\partial t} + \vec{u} \cdot \nabla p ~.
\end{equation}

An energy equation written in enthalpy carries this term on its right hand side. A weakly
compressible formulation drops it, together with the viscous dissipation, and that is the default in
the Navier-Stokes module; it is reinstated by setting
[!param](/Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics/include_pressure_work)
on the fluid heat transfer `Physics`.

The term is algebraic in the pressure and the velocity, neither of which the energy equation solves
for. It is therefore formed here as a functor and handed to a [LinearFVSource.md], rather than being
assembled by a kernel of its own.

## Assembling the term in more than one piece

For a mixture, the velocity that carries the pressure work is not the mixture velocity alone. The
phases move relative to the centre of mass, and the relative motion carries pressure work of its
own, so that the full coefficient is $\vec{u}_m + (\alpha - c_d)\vec{u}_s$ with
$c_d = \alpha\rho_d/\rho_m$ the mass fraction of the dispersed phase. That group is the volumetric
drift, the difference between the volume averaged and the mass averaged mixture velocity.

It is convenient to assemble the two contributions separately, since the mixture part is available
to a single-phase formulation and the drift part is not. Only one of the pieces may then carry the
transient part, or $\partial p/\partial t$ is counted twice.
[!param](/FunctorMaterials/NSFVPressureWorkFunctorMaterial/include_time_derivative) exists for that
purpose and is set to `false` on the drift piece.

The transient part is taken from the pressure functor's own time derivative, so that it is the same
discrete operator the transient terms of the other equations are advanced with rather than a
difference formed here. A steady problem reports no rate and the term reduces to the advective part
alone.

!syntax parameters /FunctorMaterials/NSFVPressureWorkFunctorMaterial

!syntax inputs /FunctorMaterials/NSFVPressureWorkFunctorMaterial

!syntax children /FunctorMaterials/NSFVPressureWorkFunctorMaterial

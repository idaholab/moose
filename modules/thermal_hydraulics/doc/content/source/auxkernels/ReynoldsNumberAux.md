# ReynoldsNumberAux

!syntax description /AuxKernels/ReynoldsNumberAux

The Reynolds number $Re$ is computed as:

!equation
Re = \dfrac{ \alpha * \rho * |vel| \text{hydraulic diameter}}{$\mu$}

where $\alpha$ is the phase volume fraction, $\rho$ the density, $|vel|$ the absolute value of
the component of the phase velocity that is aligned with the flow and $\mu$ the dynamic viscosity.

!alert note
This object is for use with the conserved variable set. To use temperature and pressure,
consider using the [ReynoldsNumberFunctorAux.md].

!alert note
$v$, $e$, $vel$ and $\rho$ are variables declared when using a single-phase component.

!syntax parameters /AuxKernels/ReynoldsNumberAux

!syntax inputs /AuxKernels/ReynoldsNumberAux

!syntax children /AuxKernels/ReynoldsNumberAux

# INSFEMaterial

!syntax description /Materials/INSFEMaterial

The fluid properties are computed as [material properties](Materials/index.md)
from the flow variables -- velocity, pressure, temperature -- and from the fluid properties.
The fluid properties computed are:

- density
- dynamic viscosity
- thermal diffusivity
- specific heat capacity


Additionally, several turbulence quantities are computed using a mixing length
model:

- the turbulent dynamic viscosity
- the turbulent heat conductivity
- the viscous stress tensor from both dynamic and turbulent viscosity


The stabilization terms for [PSPG/ SUPG stabilization](navier_stokes/cgfe.md) are also computed:

- the minimum dimension of the element 'hmin'
- the mass PSPG stabilization term $\tau_c$
- the momentum and energy equation SUPG stabilization $\tau$ terms


!syntax parameters /Materials/INSFEMaterial

!syntax inputs /Materials/INSFEMaterial

!syntax children /Materials/INSFEMaterial

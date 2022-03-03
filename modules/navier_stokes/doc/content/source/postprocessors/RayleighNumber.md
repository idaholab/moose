# RayleighNumber

!syntax description /Postprocessors/RayleighNumber

The Rayleigh number is computed as:

!equation
Ra = \dfrac{\Delta \rho l^3 g}{\mu k} = \dfrac{\rho \beta \Delta T l^3 g}{\mu k}

where:

- $\Delta \rho$ is the density difference between the warm and cool region
- $\Delta T$ is the temperature difference between the warm and cool region
- $\beta$ is the fluid expansion coefficient
- $l$ is the size of the system in the direction of the temperature difference
- $g$ is the magnitude of the gravity
- $\mu$ is the dynamic viscosity of the fluid
- $k$ is the thermal diffusivity of the fluid


The density difference may be provided either directly or using the fluid expansion coefficient
and the temperature difference.
All quantities but gravity may be provided as postprocessors.

!syntax parameters /Postprocessors/RayleighNumber

!syntax inputs /Postprocessors/RayleighNumber

!syntax children /Postprocessors/RayleighNumber

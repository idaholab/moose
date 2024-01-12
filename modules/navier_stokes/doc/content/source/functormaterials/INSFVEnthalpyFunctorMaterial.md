# INSFVEnthalpyFunctorMaterial

This is the material class used to compute enthalpy $\rho c_p T$ where $\rho$ is the density, $c_p$
is the specific heat capacity, and $T$ is the temperature, for the incompressible/weakly-compressible
finite-volume implementation of the Navier-Stokes equations.

It defines the following functor material properties, for a variety of use cases and postprocessing needs:

- enthalpy density $\rho h$ or $\rho c_p T$
- specific enthalpy $h$ or $c_p T$
- time derivative of the specific enthalpy $\dfrac{dh}{dt} = c_p \dfrac{dT}{dt}$

!syntax parameters /FunctorMaterials/INSFVEnthalpyFunctorMaterial

!syntax inputs /FunctorMaterials/INSFVEnthalpyFunctorMaterial

!syntax children /FunctorMaterials/INSFVEnthalpyFunctorMaterial

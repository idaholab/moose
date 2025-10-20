# INSFVEnthalpyFunctorMaterial

This is the material class used to compute enthalpy $\rho c_p T$ where $\rho$ is the density, $c_p$
is the specific heat capacity, and $T$ is the temperature, for the incompressible/weakly-compressible
finite-volume implementation of the Navier-Stokes equations.

It defines the following functor material properties, for a variety of use cases and postprocessing needs:

If [!param](/FunctorMaterials/INSFVEnthalpyFunctorMaterial/assumed_constant_cp) is set to true (default):

- enthalpy density $\rho h$ and $\rho c_p T$, named as "rho_h" and "rho_cp_temp"
- specific enthalpy $h$ and $c_p T$, named as "h" and "cp_temp"
- time derivative of the specific enthalpy $\dfrac{dh}{dt} = c_p \dfrac{dT}{dt}$, named as "dh_dt"


else if the specific enthalpy functor is specified with [!param](/FunctorMaterials/INSFVEnthalpyFunctorMaterial/h_in),
as well as the fluid properties using the [!param](/FunctorMaterials/INSFVEnthalpyFunctorMaterial/fp) parameter. This
case is used for solving with the specific enthalpy variable.

- enthalpy density $\rho h$, named as "rho_h"
- temperature of the fluid computed using the fluid properties from the specific enthalpy and pressure, named as "T_fluid"
- time derivative of the specific enthalpy $\dfrac{dh}{dt}$, named as "dh_dt"


else if [!param](/FunctorMaterials/INSFVEnthalpyFunctorMaterial/assumed_constant_cp) is set to false,
but the specific enthalpy functor is not specified:

- enthalpy density $\rho h$, named as "rho_h"
- specific enthalpy $h$, named as "h"
- time derivative of the specific enthalpy $\dfrac{dh}{dt}$, named as "dh_dt"


!syntax parameters /FunctorMaterials/INSFVEnthalpyFunctorMaterial

!syntax inputs /FunctorMaterials/INSFVEnthalpyFunctorMaterial

!syntax children /FunctorMaterials/INSFVEnthalpyFunctorMaterial

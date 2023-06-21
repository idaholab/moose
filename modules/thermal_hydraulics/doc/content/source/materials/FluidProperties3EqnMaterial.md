# FluidProperties3EqnMaterial

!syntax description /Materials/FluidProperties3EqnMaterial

Both fluid properties and several physical quantities are defined as material
properties.
The material properties defined (declared and computed) are:

- density $\rho$
- specific volume $v$
- 1D velocity $u$
- specific internal energy $e$
- pressure $p$
- temperature $T$
- specific enthalpy $h$
- specific total enthalpy $H$
- speed of sound $c$
- specific isobaric heat capacity $c_p$
- specific isochoric heat capacity $c_v$
- thermal conductivity $k$

Additionally, several derivative of material properties with regards to the conserved variables;
the conserved density $\rho A$, the conserved momentum $\rho uA$ and the conserved total energy
$\rho E A$ are defined:

- $\dfrac{d\rho}{d\rho A}$
- $\dfrac{dv}{d\rho A}$
- $\dfrac{du}{d\rho A}$
- $\dfrac{du}{d\rho u A}$
- $\dfrac{de}{d\rho A}$
- $\dfrac{de}{d\rho u A}$
- $\dfrac{de}{d\rho E A}$
- $\dfrac{dp}{d\rho A}$
- $\dfrac{dp}{d\rho u A}$
- $\dfrac{dp}{d\rho E A}$
- $\dfrac{dT}{d\rho A}$
- $\dfrac{dT}{d\rho u A}$
- $\dfrac{dT}{d\rho E A}$
- $\dfrac{dh}{d\rho A}$
- $\dfrac{dh}{d\rho u A}$
- $\dfrac{dh}{d\rho E A}$
- $\dfrac{dH}{d\rho A}$
- $\dfrac{dH}{d\rho u A}$
- $\dfrac{dH}{d\rho E A}$

!syntax parameters /Materials/FluidProperties3EqnMaterial

!syntax inputs /Materials/FluidProperties3EqnMaterial

!syntax children /Materials/FluidProperties3EqnMaterial

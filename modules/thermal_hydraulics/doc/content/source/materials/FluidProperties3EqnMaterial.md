# FluidProperties3EqnMaterial

!syntax description /Materials/FluidProperties3EqnMaterial

Both fluid properties and several physical quantities are defined as material
properties.
The material properties defined (declared and computed) are:

- density $\rho$
- specific volume $v$
- 1D velocity $vel$
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
the conserved density $\rhoA$, the conserved momentum $\rho uA$ and the conserved total energy
$\rho E A$ are defined:

- $\dfrac{d\rho}{d\rhoA}
- $\dfrac{dv}{d\rhoA}
- $\dfrac{dvel}{d\rhoA}
- $\dfrac{dvel}{d\rhouA}
- $\dfrac{de}{d\rhoA}
- $\dfrac{de}{d\rhouA}
- $\dfrac{de}{d\rhoEA}
- $\dfrac{dp}{d\rhoA}
- $\dfrac{dp}{d\rhouA}
- $\dfrac{dp}{d\rhoEA}
- $\dfrac{dT}{d\rhoA}
- $\dfrac{dT}{d\rhouA}
- $\dfrac{dT}{d\rhoEA}
- $\dfrac{dh}{d\rhoA}
- $\dfrac{dh}{d\rhouA}
- $\dfrac{dh}{d\rhoEA}
- $\dfrac{dH}{d\rhoA}
- $\dfrac{dH}{d\rhouA}
- $\dfrac{dH}{d\rhoEA}

!syntax parameters /Materials/FluidProperties3EqnMaterial

!syntax inputs /Materials/FluidProperties3EqnMaterial

!syntax children /Materials/FluidProperties3EqnMaterial

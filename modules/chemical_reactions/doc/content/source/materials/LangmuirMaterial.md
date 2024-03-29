# LangmuirMaterial

!syntax description /Materials/LangmuirMaterial

This material computes the mass rates from the matrix to the porespace, as well as the derivatives
of the mass rates with regards to the gas species and with regards to the pressure.

If the concentration is above the equilibrium concentration, desorption occurs and the mass rate is
equal to

!equation
\dot{m}_{m\rightarrow p} = \dfrac{C-C_{eq}}{K_d} 

else adsorption is happening and:

!equation
\dot{m}_{m\rightarrow p} = \dfrac{C-C_{eq}}{K_a} 

where $m_{m\rightarrow p}$ is the mass rate from the matrix to the porespace, $C$ the species concentration, $C_{eq}$ the
equilibrium concentration and $K_d$ / $K_a$ are the desorption and adsorption time constants (in seconds).

!syntax parameters /Materials/LangmuirMaterial

!syntax inputs /Materials/LangmuirMaterial

!syntax children /Materials/LangmuirMaterial

# MollifiedLangmuirMaterial

!syntax description /Materials/MollifiedLangmuirMaterial

This material computes the mollified mass rates from the matrix to the porespace, as well as the derivatives
of the mollified mass rates with regards to the gas species and with regards to the pressure.

If the concentration is above the equilibrium concentration, desorption occurs and the mass rate is
equal to

!equation
\dot{m}_{m\rightarrow p} = A_{mol} * \dfrac{C-C_{eq}}{K_d} 

else adsorption is happening and:

!equation
\dot{m}_{m\rightarrow p} = A_{mol} * \dfrac{C-C_{eq}}{K_a} 

where $m_{m\rightarrow p}$ is the mass rate from the matrix to the porespace, $C$ the species concentration, $C_{eq}$ the
equilibrium concentration and $K_d$ / $K_a$ are the desorption and adsorption time constants (in seconds). The mollifier term
$A_{mol}$ is equal to

!equation
A_{mol} = tanh(\dfrac{C - C_{eq}}{ \text{mollifier} * \text{Langmuir density}})

!syntax parameters /Materials/MollifiedLangmuirMaterial

!syntax inputs /Materials/MollifiedLangmuirMaterial

!syntax children /Materials/MollifiedLangmuirMaterial

# DittusBoelterFunctorMaterial

!syntax description /FunctorMaterials/DittusBoelterFunctorMaterial

The Dittus-Boelter heat transfer coefficient $h$ is computed as:

!equation
h = \dfrac{Nu k}{D_h}

with $D_h$ the local hydraulic diameter, $k$ the thermal conductivity and $Nu$ the Nusselt number.
The Nusselt number in this correlation is:

!equation
Nu = 0.023 Re^{\dfrac{4}{5}} Pr^n;

with $n$ = 0.4 if the fluid temperature is lower than the wall temperature and 0.3 otherwise,
$Re$ the Reynolds number and $Pr$ the Prandtl number.

!syntax parameters /FunctorMaterials/DittusBoelterFunctorMaterial

!syntax inputs /FunctorMaterials/DittusBoelterFunctorMaterial

!syntax children /FunctorMaterials/DittusBoelterFunctorMaterial

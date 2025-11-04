# DittusBoelterFunctorMaterial

The Dittus-Boelter heat transfer coefficient $h$ is computed as:

!equation
h = \dfrac{Nu k}{D_h}

where $D_h$ is the local hydraulic diameter, $k$ is the thermal conductivity and $Nu$ is the Nusselt number.
The Nusselt number in this correlation is:

!equation
Nu = 0.023 Re^{\dfrac{4}{5}} Pr^n;

with $n = 0.4$ if the fluid temperature is lower than the wall temperature and $n = 0.3$ otherwise,
$Re$ being the Reynolds number and $Pr$ being the Prandtl number.

This correlation is valid for Reynolds numbers in the range $10^4 \leq \text{Re} \leq 1.2 \cdot 10^5$ and Prandtl numbers in the range $0.7 \leq \text{Pr} \leq 160$.

!syntax parameters /FunctorMaterials/DittusBoelterFunctorMaterial

!syntax inputs /FunctorMaterials/DittusBoelterFunctorMaterial

!syntax children /FunctorMaterials/DittusBoelterFunctorMaterial

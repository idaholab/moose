# WallHeatTransferCoefficient3EqnDittusBoelterMaterial

!syntax description /Materials/WallHeatTransferCoefficient3EqnDittusBoelterMaterial

The Dittus-Boelter heat transfer coefficient $h$ is computed as:

!equation
h = \dfrac{Nu k}{D_h}

with $D_h$ the local hydraulic diameter, $k$ the thermal conductivity and $Nu$ the Nusselt number.
The Nusselt number in this correlation is:

!equation
Nu = 0.023 Re^{\dfrac{4}{5}} Pr^n;

with $n$ = 0.4 if the fluid temperature is greater than the wall temperature and 0.3 otherwise,
$Re$ the Reynolds number and $Pr$ the Prandtl number.

!syntax parameters /Materials/WallHeatTransferCoefficient3EqnDittusBoelterMaterial

!syntax inputs /Materials/WallHeatTransferCoefficient3EqnDittusBoelterMaterial

!syntax children /Materials/WallHeatTransferCoefficient3EqnDittusBoelterMaterial

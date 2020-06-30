# INSADEnergyWallConvection

This object adds a term of the incompressible energy equation of the form
$\alpha \left(T - T_{wall}\right)$ where $\alpha$ is based on the `Real` `alpha`
parameter set by the user and $T_{wall}$ is similarly set through the `Real`
`T_wall` parameter. This term addition is meant to represent a volumetric
approximation of an energy exchange between an external heat exchanger or some
other ambient surrounding heat source/sink.

!syntax description /Kernels/INSADEnergyWallConvection

!syntax parameters /Kernels/INSADEnergyWallConvection

!syntax inputs /Kernels/INSADEnergyWallConvection

!syntax children /Kernels/INSADEnergyWallConvection

# INSADEnergyAmbientConvection

This object adds a term of the incompressible energy equation of the form
$\alpha \left(T - T_{ambient}\right)$ where $\alpha$ is based on the `Real` `alpha`
parameter set by the user and $T_{ambient}$ is similarly set through the `Real`
`T_ambient` parameter. This term addition is meant to represent a volumetric
approximation of an energy exchange between an external heat exchanger or some
other ambient surrounding heat source/sink.

!syntax description /Kernels/INSADEnergyAmbientConvection

!syntax parameters /Kernels/INSADEnergyAmbientConvection

!syntax inputs /Kernels/INSADEnergyAmbientConvection

!syntax children /Kernels/INSADEnergyAmbientConvection

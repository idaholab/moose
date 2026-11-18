# FluidPropertiesBinaryDiffusionCoefMaterial

This material computes the mass diffusion coefficient $D$ of the primary and secondary gases of a
binary gas mixture, as required by [FlowChannelGasMix.md], from the mixture pressure and
temperature with the `binaryDiffusionCoefficient` interface of its
[fluid properties](VaporMixtureFluidProperties.md).
[IdealGasMixtureFluidProperties.md] implements this interface for components of type
[NASA7IdealGasFluidProperties.md], with Chapman-Enskog kinetic theory. Unlike
[BinaryDiffusionCoefMaterial.md], this material needs no collision diameters as input.

For example, for a flow channel `pipe` that carries a mixture of steam and hydrogen:

!listing thermal_hydraulics/test/tests/components/flow_channel_gasmix/flow_channel_gasmix_nasa7.i block=FluidProperties Materials

!syntax parameters /Materials/FluidPropertiesBinaryDiffusionCoefMaterial

!syntax inputs /Materials/FluidPropertiesBinaryDiffusionCoefMaterial

!syntax children /Materials/FluidPropertiesBinaryDiffusionCoefMaterial

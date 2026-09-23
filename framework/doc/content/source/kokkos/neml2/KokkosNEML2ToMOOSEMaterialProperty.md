# KokkosNEML2ToMOOSEMaterialProperty

!if! function=hasCapability('kokkos & neml2')

Reads a NEML2 output into a Kokkos material property without transferring it through host memory.

This is the Kokkos counterpart of
[NEML2ToMOOSEMaterialProperty](NEML2ToMOOSEMaterialProperty.md). The non-Kokkos object slices one
batch entry per quadrature point and copies each into host memory, which costs one device-to-host
transfer per quadrature point per property. This object instead aliases the storage of the NEML2
output tensor and gathers the components on the device, so the whole scatter is a single kernel and
no values cross the bus.

The property type must have the same component layout as the NEML2 variable. `Real6` corresponds to
`SR2` and `Real66` to `SSR4`: both are Mandel with the component order of
[SymmetricRankTwoTensor](SymmetricRankTwoTensor.md), so the components are copied without
conversion. Two variants are registered accordingly:

- `KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty`, emitting `Real6`
- `KokkosNEML2ToMOOSESymmetricRankFourTensorMaterialProperty`, emitting `Real66`

!alert note
Aliasing requires the NEML2 output to be contiguous and resident on the device, so the NEML2
[!param](/NEML2/device) and `output_device` must name a CUDA device. A variable that is not
contiguous, which a derivative retrieved through an expanding view can be, is rejected rather than
read through the wrong strides.

## Example Input Syntax

!listing solid_mechanics/test/tests/kokkos/neml2_bridge/bridge.i block=Materials/kokkos_stress

!syntax parameters /Materials/KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty

!syntax inputs /Materials/KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty

!syntax children /Materials/KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty

!if-end!

!else
!include kokkos/kokkos_warning.md

# KokkosNEML2ToMOOSEMaterialProperty

!if! function=hasCapability('kokkos & neml2')

Reads a NEML2 output into a Kokkos material property without transferring it through host memory.

This is the Kokkos counterpart of
[NEML2ToMOOSEMaterialProperty](NEML2ToMOOSEMaterialProperty.md). The non-Kokkos object slices one
batch entry per quadrature point and copies each into host memory, which costs one device-to-host
transfer per quadrature point per property. This object instead aliases the storage of the NEML2
output tensor and gathers the components on the device, so the whole scatter is a single kernel and
no values cross the bus.

The property type must have the same component layout as the NEML2 variable. `Real` corresponds to a
`Scalar`, `Real6` to `SR2` and `Real66` to `SSR4`: the latter two are Mandel with the component order
of [SymmetricRankTwoTensor](SymmetricRankTwoTensor.md), so the components are copied without
conversion. Three variants are registered accordingly:

- `KokkosNEML2ToMOOSERealMaterialProperty`, emitting `Real`
- `KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty`, emitting `Real6`
- `KokkosNEML2ToMOOSESymmetricRankFourTensorMaterialProperty`, emitting `Real66`

Retrieving a stateful NEML2 output into a Kokkos property is what allows the model's old state to be
gathered back on the device by [KokkosMOOSEQuantityToNEML2](KokkosMOOSEQuantityToNEML2.md), keeping
the whole state round trip off the host.

!alert note
Aliasing requires contiguous storage, and a NEML2 output is not always contiguous: a state variable
is a slice of the model's state axis, and a derivative is a broadcast. Such an output is compacted on
the device first, at the cost of one device-to-device copy per solve. The output must still be
resident on the device, so the NEML2 [!param](/NEML2/device) and `output_device` must name a CUDA
device.

## Example Input Syntax

!listing solid_mechanics/test/tests/kokkos/neml2_bridge/bridge.i block=Materials/kokkos_stress

!syntax parameters /Materials/KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty

!syntax inputs /Materials/KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty

!syntax children /Materials/KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty

!if-end!

!else
!include kokkos/kokkos_warning.md

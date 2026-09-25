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

Supplying [!param](/Materials/KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty/neml2_input_derivative)
reads the derivative of the output with respect to that input instead of the output itself, which is how
a model's tangent is obtained.

## Storage of an Output That Does Not Vary

A derivative that is the same for every batch entry, such as the tangent of a linear elastic model,
arrives from NEML2 as a broadcast: one set of components that every entry aliases, with a batch stride
of zero. Compacting it in order to alias it would duplicate those components once per quadrature point,
so the single set is aliased instead and every quadrature point reads it.

Setting `constant_on = SUBDOMAIN` on this material additionally stores the property once per subdomain
rather than once per quadrature point, which for a rank-four tensor is the difference between 648 bytes
and 648 bytes per quadrature point in the mesh. That is only faithful to an output NEML2 broadcasts, so
storing an output that does vary along the batch more coarsely than per quadrature point is reported as
an error rather than recording one arbitrary quadrature point's value.

!alert note
Aliasing requires contiguous storage, and a NEML2 output is not always contiguous. A state variable is
a slice of the model's state axis, so it is compacted on the device, which copies exactly the values
needed. A broadcast is aliased as described above. The output must in either case be resident on the
device, so the NEML2 [!param](/NEML2/device) and `output_device` must name a CUDA device.

## Example Input Syntax

!listing solid_mechanics/test/tests/kokkos/neml2_bridge/bridge.i block=Materials/kokkos_stress

Reading a tangent, stored once per subdomain:

!listing solid_mechanics/test/tests/kokkos/neml2_bridge/tangent.i block=Materials/kokkos_jacobian

!syntax parameters /Materials/KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty

!syntax inputs /Materials/KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty

!syntax children /Materials/KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty

!if-end!

!else
!include kokkos/kokkos_warning.md

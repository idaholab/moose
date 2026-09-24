# KokkosMOOSEQuantityToNEML2

!if! function=hasCapability('kokkos & neml2')

Gathers a Kokkos material property into a NEML2 input variable without transferring it through host
memory.

This is the Kokkos counterpart of [MOOSEQuantityToNEML2](MOOSEQuantityToNEML2.md). The non-Kokkos
object copies each quadrature point's value into a host buffer during an element loop and transfers
the buffer once per solve. This object fills a device buffer with a single Kokkos kernel and hands
NEML2 a tensor aliasing that buffer, so the values never cross the bus.

A NEML2 model is evaluated pointwise along its batch dimension, so which quadrature point occupies
which batch entry is a free choice, constrained only by every object gathering an input and every
object retrieving an output agreeing on it. This object and
[KokkosNEML2ToMOOSEMaterialProperty](KokkosNEML2ToMOOSEMaterialProperty.md) share one definition of
that numbering, taken from the numbering Kokkos material property storage already uses.

The property type must have the same component layout as the NEML2 variable. `Real` corresponds to a
NEML2 `Scalar`, and `Real6` to `SR2`, which is Mandel with the component order of
[SymmetricRankTwoTensor](SymmetricRankTwoTensor.md), so the components are copied without conversion.
Four variants are registered accordingly, in current-value and old-value pairs:

- `KokkosMOOSERealToNEML2` and `KokkosMOOSEOldRealToNEML2`, reading `Real`
- `KokkosMOOSESymmetricRankTwoTensorToNEML2` and `KokkosMOOSEOldSymmetricRankTwoTensorToNEML2`,
  reading `Real6`

A NEML2 model with history takes its old state as an input. Pairing the `Old` variants with
[KokkosNEML2ToMOOSEMaterialProperty](KokkosNEML2ToMOOSEMaterialProperty.md) closes that round trip on
the device: each stateful output is retrieved into a Kokkos material property, and its old value is
gathered back as the corresponding input. Requesting the old value is also what makes the property
stateful, so no separate declaration is needed.

Name the object after the NEML2 input variable it supplies and list that variable in
[!param](/NEML2/input_kernels), so that the `NEML2` action does not also create a host gatherer for
it.

Kokkos user objects are computed after the Kokkos materials they consume and before the non-Kokkos
user objects, so the property this object reads is current and the NEML2 executor sees the gathered
value.

## Example Input Syntax

!listing solid_mechanics/test/tests/kokkos/neml2_gather/gather.i block=UserObjects/neml2_strain

!syntax parameters /UserObjects/KokkosMOOSESymmetricRankTwoTensorToNEML2

!syntax inputs /UserObjects/KokkosMOOSESymmetricRankTwoTensorToNEML2

!syntax children /UserObjects/KokkosMOOSESymmetricRankTwoTensorToNEML2

!if-end!

!else
!include kokkos/kokkos_warning.md

# Compute Reduced Order Eigenstrain

!syntax description /Materials/ComputeReducedOrderEigenstrain

## Description

Since strain is a function of the derivative of displacements, the strain field is one order lower than the displacement field.  If the element is linear, the strain field will be constant; if the element is quadratic, the strain field will be linear.

This being the case, eigenstrains that affect the strain need to be of the same order as the strain.  A thermal strain taken from temperatures at nodes will vary according to the order of the temperature and not the strain field.  Using a thermal strain one order higher than the strain field can lead to oscillations in the overall strain and stress.

This class accepts eigenstrains and computes a reduced order eigenstrain.  If the primary solution variable field is linear, the resulting eigenstrain will be constant in an element using volume weighting.  If the primary solution variable field is quadratic, the resulting eigenstrain will be linear in an element using a least squares procedure.

!alert warning When using the
[`ComputeReducedOrderEigenstrain`](ComputeReducedOrderEigenstrain.md) object for
RankTwoTensor eigenstrains with the
[TensorMechanicsAction](TensorMechanics/Master/index.md) setting
`automatic_eigenstrain_names = true`, eigenstrains listed as MaterialADConverter
input  tensors will not be included in the `eigenstrain_names` list passed. Set
the automatic/_eigenstrain/_names = false and populate this list manually if
these components need to be included.

## Example Input File syntax

!listing modules/tensor_mechanics/test/tests/eigenstrain/reducedOrderRZLinear.i block=Materials/reduced_order_eigenstrain

The `eigenstrain_name` parameter value must also be set for the strain calculator, and an example parameter setting is shown below:

!listing modules/tensor_mechanics/test/tests/eigenstrain/reducedOrderRZLinear.i block=Modules

!syntax parameters /Materials/ComputeReducedOrderEigenstrain

!syntax inputs /Materials/ComputeReducedOrderEigenstrain

!syntax children /Materials/ComputeReducedOrderEigenstrain

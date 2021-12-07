# MaterialADConverter

The `MaterialADConverter` is used to explicitly convert regular material
properties into AD material properties and visa versa.

!alert warning When using the [`MaterialADConverter`](MaterialADConverter.md) object
for RankTwoTensor eigenstrains with the
`TensorMechanicsAction` setting
`automatic_eigenstrain_names = true`, eigenstrains listed as MaterialADConverter
input  tensors will not be included in the `eigenstrain_names` list passed. Set
the automatic/_eigenstrain/_names = false and populate this list manually if
these components need to be included.

## Description and Syntax

!syntax description /Materials/MaterialADConverter

!syntax parameters /Materials/MaterialADConverter

!syntax inputs /Materials/MaterialADConverter

!syntax children /Materials/MaterialADConverter

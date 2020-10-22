# MaterialConverter

The `MaterialConverter` is used to explicitly convert regular material
properties into AD material properties and visa versa.

!alert warning When using the [`MaterialConverter`](MaterialConverter.md) object
for RankTwoTensor eigenstrains with the
`TensorMechanicsAction` setting
`automatic_eigenstrain_names = true`, eigenstrains listed as MaterialConverter
input  tensors will not be included in the `eigenstrain_names` list passed. Set
the automatic/_eigenstrain/_names = false and populate this list manually if
these components need to be included.

## Description and Syntax

!syntax description /Materials/MaterialConverter

!syntax parameters /Materials/MaterialConverter

!syntax inputs /Materials/MaterialConverter

!syntax children /Materials/MaterialConverter

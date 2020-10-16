# MaterialConverter

The `MaterialConverter` is used to explicitly convert regular material
properties into AD material properties and visa versa.

!alert warning If the `MaterialConverter` for RankTwoTensor eigenstrains, using
the TensorMechanicsAction with automatic/_eigenstrain/_names = true, the input
tensors/eigenstrains will not be included in the eigenstrain/_names list passed.
Set the automatic/_eigenstrain/_names = false and populate this list manually if
these components need to be included.

## Description and Syntax

!syntax description /Materials/MaterialConverter

!syntax parameters /Materials/MaterialConverter

!syntax inputs /Materials/MaterialConverter

!syntax children /Materials/MaterialConverter

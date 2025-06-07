# MOOSEMaterialPropertyToNEML2

!alert note
Users are +NOT+ expected to directly use this object in an input file. Instead, it is always recommended to use the [NEML2 action](syntax/NEML2/index.md).

## Description

This family of objects collect a MOOSE material property given by [!param](/UserObjects/MOOSERealMaterialPropertyToNEML2/from_moose) for use as a NEML2 input variable or model parameter [!param](/UserObjects/MOOSERealMaterialPropertyToNEML2/to_neml2).

The following flavors exist:

| Class                                                | MOOSE MaterialProperty type |
| :--------------------------------------------------- | :-------------------------- |
| `MOOSERealMaterialPropertyToNEML2`                   | `Real`                      |
| `MOOSERankTwoTensorMaterialPropertyToNEML2`          | `RankTwoTensor`             |
| `MOOSESymmetricRankTwoTensorMaterialPropertyToNEML2` | `SymmetricRankTwoTensor`    |
| `MOOSEStdVectorRealMaterialPropertyToNEML2`          | `std::vector<Real>`         |

Each class has an "old" counterpart to retrieve the corresponding MOOSE material property from the previous time step. The naming convention is

```
MOOSE[Old]<Type>MaterialPropertyToNEML2
```

For example, `MOOSEOldRealMaterialPropertyToNEML2` gathers the `Real`-valued material property from the previous time step.

!syntax parameters /UserObjects/MOOSERealMaterialPropertyToNEML2

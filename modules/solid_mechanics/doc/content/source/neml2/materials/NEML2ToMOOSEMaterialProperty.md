# NEML2ToMOOSEMaterialProperty

!alert note
Users are +NOT+ expected to directly use this object in an input file. Instead, it is always recommended to use the [NEML2 action](syntax/NEML2/index.md).

## Description

This family of objects assign a NEML2 output variable given by [!param](/Materials/NEML2ToRealMOOSEMaterialProperty/neml2_variable) back to the quadrature points.

Note that if the [!param](/Materials/NEML2ToRealMOOSEMaterialProperty/neml2_input_derivative) parameter is specified, the derivative of the NEML2 output variable with respect to the input variable is retrieved instead. If the [!param](/Materials/NEML2ToRealMOOSEMaterialProperty/neml2_parameter_derivative) parameter is specified, the derivative of the NEML2 output variable with respect to the model parameter is retrieved instead.

The following flavors exist:

| Class                                                 | MOOSE MaterialProperty type |
| :---------------------------------------------------- | :-------------------------- |
| `NEML2ToRealMOOSEMaterialProperty`                    | `Real`                      |
| `NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty`  | `SymmetricRankTwoTensor`    |
| `MOOSESymmetricRankTwoTensorMaterialPropertyToNEML2`  | `SymmetricRankFourTensor`   |
| `NEML2ToSymmetricRankFourTensorMOOSEMaterialProperty` | `std::vector<Real>`         |

!syntax parameters /Materials/NEML2ToRealMOOSEMaterialProperty

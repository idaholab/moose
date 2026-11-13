# NEML2ToMOOSEMaterialProperty

!if! function=hasCapability('neml2')

!alert note
Users are +NOT+ expected to directly use this object in an input file. Instead, it is always recommended to use the [NEML2 action](syntax/NEML2/index.md).

## Description

This family of objects assign a NEML2 output variable given by [!param](/Materials/NEML2ToMOOSERealMaterialProperty/from_neml2) back to the quadrature points. The data is stored as a MOOSE material property with name given by [!param](/Materials/NEML2ToMOOSERealMaterialProperty/to_moose).

Note that if the [!param](/Materials/NEML2ToMOOSERealMaterialProperty/neml2_input_derivative) parameter is specified, the derivative of the NEML2 output variable with respect to the input variable is retrieved instead. If the [!param](/Materials/NEML2ToMOOSERealMaterialProperty/neml2_parameter_derivative) parameter is specified, the derivative of the NEML2 output variable with respect to the model parameter is retrieved instead.

The following flavors exist:

| Class                                                 | MOOSE MaterialProperty type |
| :---------------------------------------------------- | :-------------------------- |
| `NEML2ToMOOSERealMaterialProperty`                    | `Real`                      |
| `NEML2ToMOOSESymmetricRankTwoTensorMaterialProperty`  | `SymmetricRankTwoTensor`    |
| `NEML2ToMOOSESymmetricRankFourTensorMaterialProperty` | `SymmetricRankFourTensor`   |
| `NEML2ToMOOSEStdVectorMaterialProperty`               | `std::vector<Real>`         |

In element interiors, the object reads the NEML2 batch span of the current element. On a side, it
reads the span of the current `(element, side)` pair. If that location was not evaluated, or if the
span size does not match the current quadrature rule, the object stops with an error rather than
reading the wrong entries. See [evaluation domains](syntax/NEML2/index.md#domains) for how to select
the evaluated locations.

!syntax parameters /Materials/NEML2ToMOOSERealMaterialProperty

!if-end!

!else

!include neml2/neml2_warning.md

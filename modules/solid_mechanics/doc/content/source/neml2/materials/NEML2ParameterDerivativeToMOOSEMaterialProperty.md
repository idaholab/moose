# NEML2ParameterDerivativeToMOOSEMaterialProperty

This object requests the computation of the derivative of a NEML2 output variable with respect to a NEML2 parameter and saves the derivative as a MOOSE MaterialProperty. The derivative is computed in [ExecuteNEML2Model](ExecuteNEML2Model.md).

This object utilizes the automatic differentiation (AD) feature in NEML2. To ensure proper functionality, make sure that `enable_AD = true` is set in the `ExecuteNEML2Model` block when using this object.

!alert note title=Note
Only `Scalar` NEML2 parameters are supported in this case, which keeps the type of the derivative consistent with that of the NEML2 variable. The supported object types are:

!table caption=`NEML2ParameterDerivativeToMOOSEMaterialProperty` objects
| Object name | Moose MaterialProperty type |
| - | - |
| `NEML2ParameterDerivativeToRealMOOSEMaterialProperty`  | `Real` |
| `NEML2ParameterDerivativeToStdVectorRealMOOSEMaterialProperty` | `std::vector<Real>` |
| `NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty` | `SymmetricRankTwoTensor` |
| `NEML2ParameterDerivativeToSymmetricRankFourTensorMOOSEMaterialProperty` | `SymmetricRankFourTensor` |


## Example Input Syntax

!listing modules/solid_mechanics/test/tests/neml2/moose_to_neml2_parameters.i block=Materials/dstress_dE

!syntax parameters /Materials/NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty

!syntax inputs /Materials/NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty

!syntax children /Materials/NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty

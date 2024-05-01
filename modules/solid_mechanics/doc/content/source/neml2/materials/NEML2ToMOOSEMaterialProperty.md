# NEML2ToMOOSEMaterialProperty

!syntax description /Materials/NEML2ToRealMOOSEMaterialProperty

## Description

Objects in this family assign a selected NEML2 model output computed through [ExecuteNEML2Model](ExecuteNEML2Model.md) to a MOOSE MaterialProperty. The following flavors exist:

!table caption=`NEML2ToMOOSEMaterialProperty` objects
| Object name | MOOSE material property type |
| - | - |
| `NEML2ToRealMOOSEMaterialProperty`  | `Real` |
| `NEML2ToStdVectorRealMOOSEMaterialProperty` | `std::vector<Real>` |
| `NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty` | `SymmetricRankTwoTensor` |
| `NEML2ToSymmetricRankFourTensorMOOSEMaterialProperty`  |`SymmetricRankFourTensor` |

## Example Input Syntax


!syntax parameters /Materials/NEML2ToRealMOOSEMaterialProperty

!syntax inputs /Materials/NEML2ToRealMOOSEMaterialProperty

!syntax children /Materials/NEML2ToRealMOOSEMaterialProperty

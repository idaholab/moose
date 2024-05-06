# MOOSEMaterialPropertyToNEML2

!syntax description /UserObjects/MOOSERealMaterialPropertyToNEML2

## Description

This family of objects collects a given [!param](/UserObjects/MOOSERealMaterialPropertyToNEML2/moose_material_property) for use as an input [!param](/UserObjects/MOOSERealMaterialPropertyToNEML2/neml2_variable) to a NEML2 model executed by a [ExecuteNEML2Model](ExecuteNEML2Model.md) user object.

The following flavors exist:

!table caption=`MOOSEMaterialPropertyToNEML2` objects
| Object nam | Moose MaterialProperty type |
| - | - |
| `MOOSERealMaterialPropertyToNEML2`  | `Real` |
| `MOOSEStdVectorRealMaterialPropertyToNEML2` | `std::vector<Real>` |
| `MOOSERankTwoTensorMaterialPropertyToNEML2` | `RankTwoTensor` |

To use a MOOSE +variable+ as input to a NEML2 object see [MOOSEVariableToNEML2](MOOSEVariableToNEML2.md).

## Example Input Syntax

!listing modules/solid_mechanics/test/tests/neml2/neml2_to_moose_material.i block=UserObjects/gather_b

!syntax parameters /UserObjects/MOOSERealMaterialPropertyToNEML2

!syntax inputs /UserObjects/MOOSERealMaterialPropertyToNEML2

!syntax children /UserObjects/MOOSERealMaterialPropertyToNEML2

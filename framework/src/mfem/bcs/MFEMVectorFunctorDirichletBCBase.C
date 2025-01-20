#include "MFEMVectorFunctorDirichletBCBase.h"

InputParameters
MFEMVectorFunctorDirichletBCBase::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<platypus::MFEMVectorCoefficientName>(
      "vector_coefficient",
      "The vector coefficient specifying the values variable takes on the boundary.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctorDirichletBCBase::MFEMVectorFunctorDirichletBCBase(
    const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _vec_coef_name(getParam<platypus::MFEMVectorCoefficientName>("vector_coefficient")),
    _vec_coef(getVectorProperty(_vec_coef_name))
{
}

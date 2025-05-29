#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorDirichletBCBase.h"

InputParameters
MFEMVectorFunctorDirichletBCBase::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "vector_coefficient",
      "Vector coefficient specifying the values variable takes on the boundary. A coefficient "
      "can be any of the following: a variable, an MFEM material property, a function, a "
      "post-processor, or a numerical value.");
  return params;
}

MFEMVectorFunctorDirichletBCBase::MFEMVectorFunctorDirichletBCBase(
    const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _vec_coef_name(getParam<MFEMVectorCoefficientName>("vector_coefficient")),
    _vec_coef(getVectorCoefficient(_vec_coef_name))
{
}

#endif

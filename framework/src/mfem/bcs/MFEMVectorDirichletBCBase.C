#ifdef MFEM_ENABLED

#include "MFEMVectorDirichletBCBase.h"

InputParameters
MFEMVectorDirichletBCBase::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "vector_coefficient",
      "Vector coefficient specifying the values variable takes on the boundary. A coefficient "
      "can be any of the following: a variable, an MFEM material property, a function, a "
      "post-processor, or a numerical value.");
  return params;
}

MFEMVectorDirichletBCBase::MFEMVectorDirichletBCBase(
    const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _vec_coef_name(getParam<MFEMVectorCoefficientName>("vector_coefficient")),
    _vec_coef(getVectorCoefficient(_vec_coef_name))
{
}

#endif

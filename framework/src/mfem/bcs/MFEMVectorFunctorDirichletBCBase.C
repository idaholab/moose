#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorDirichletBCBase.h"

InputParameters
MFEMVectorFunctorDirichletBCBase::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "vector_coefficient",
      "Vector functor specifying the values variable takes on the boundar. A functor is any of the "
      "following: a variable, an MFEM material property, a function, or a post-processor.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctorDirichletBCBase::MFEMVectorFunctorDirichletBCBase(
    const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _vec_coef_name(getParam<MFEMVectorCoefficientName>("vector_coefficient")),
    _vec_coef(getVectorCoefficient(_vec_coef_name))
{
}

#endif

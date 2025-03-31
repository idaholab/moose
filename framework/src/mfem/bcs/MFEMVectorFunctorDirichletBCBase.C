#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorDirichletBCBase.h"

InputParameters
MFEMVectorFunctorDirichletBCBase::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<std::string>(
      "vector_coefficient",
      "The vector coefficient specifying the values variable takes on the boundary.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctorDirichletBCBase::MFEMVectorFunctorDirichletBCBase(
    const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _vec_coef(getMFEMProblem().getProperties().getVectorProperty(
        getParam<std::string>("vector_coefficient")))
{
}

#endif

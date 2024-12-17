#ifdef MFEM_ENABLED

#include "MFEMVectorFunctionDirichletBCBase.h"
#include "MFEMProblem.h"

InputParameters
MFEMVectorFunctionDirichletBCBase::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<std::string>(
      "vector_coefficient",
      "The vector coefficient specifying the values variable takes on the boundary.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctionDirichletBCBase::MFEMVectorFunctionDirichletBCBase(
    const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _vec_coef(getMFEMProblem().getProperties().getVectorProperty(
        getParam<std::string>("vector_coefficient")))
{
}

#endif

#include "MFEMVectorFunctionDirichletBCBase.h"

InputParameters
MFEMVectorFunctionDirichletBCBase::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<FunctionName>("function",
                                        "The values the components must take on the boundary.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctionDirichletBCBase::MFEMVectorFunctionDirichletBCBase(
    const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _vec_coef(getMFEMProblem().getVectorFunctionCoefficient(getParam<FunctionName>("function")))
{
}

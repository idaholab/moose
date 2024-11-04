#include "MFEMVectorFunctionCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMVectorFunctionCoefficient);

InputParameters
MFEMVectorFunctionCoefficient::validParams()
{
  InputParameters params = MFEMVectorCoefficient::validParams();
  params.addParam<FunctionName>("function", 0, "The function to associated with the Dirichlet BC");
  return params;
}

MFEMVectorFunctionCoefficient::MFEMVectorFunctionCoefficient(const InputParameters & parameters)
  : MFEMVectorCoefficient(parameters),
    _vector_coefficient(
        getMFEMProblem().getVectorFunctionCoefficient(getParam<FunctionName>("function")))
{
}

MFEMVectorFunctionCoefficient::~MFEMVectorFunctionCoefficient() {}

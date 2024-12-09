#include "MFEMFunctionCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMFunctionCoefficient);

InputParameters
MFEMFunctionCoefficient::validParams()
{
  InputParameters params = MFEMCoefficient::validParams();
  params.addClassDescription(
      "Class for defining an mfem::FunctionCoefficient object to add to an MFEMProblem.");
  params.addParam<FunctionName>("function", 0, "The function to associated with the Dirichlet BC");
  return params;
}

MFEMFunctionCoefficient::MFEMFunctionCoefficient(const InputParameters & parameters)
  : MFEMCoefficient(parameters),
    _coefficient(getMFEMProblem().getScalarFunctionCoefficient(getParam<FunctionName>("function")))
{
}

MFEMFunctionCoefficient::~MFEMFunctionCoefficient() {}

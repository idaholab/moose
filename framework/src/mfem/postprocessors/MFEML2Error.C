#ifdef MFEM_ENABLED

#include "MFEML2Error.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEML2Error);

InputParameters
MFEML2Error::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params.addClassDescription(
      "Computes L2 error $\\left\\Vert u_{ex} - u_{h}\\right\\Vert_{\rm L2}$ for "
      "gridfucntions using H1 or L2 elements.");
  params.addParam<FunctionName>("function", "The analytic solution to compare against.");
  params.addParam<VariableName>("variable",
                                "Name of the variable of which to find the norm of the error.");
  return params;
}

MFEML2Error::MFEML2Error(const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    _var_name(getParam<VariableName>("variable")),
    _coeff_name(getParam<FunctionName>("function")),
    _coeff(getScalarCoefficient(_coeff_name)),
    _var(getMFEMProblem().getProblemData().gridfunctions.GetRef(_var_name))
{
}

void
MFEML2Error::initialize()
{
}

void
MFEML2Error::execute()
{
}

PostprocessorValue
MFEML2Error::getValue() const
{
  return _var.ComputeL2Error(_coeff);
}

#endif

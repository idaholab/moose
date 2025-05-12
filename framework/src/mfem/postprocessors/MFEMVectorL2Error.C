#ifdef MFEM_ENABLED

#include "MFEMVectorL2Error.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorL2Error);

InputParameters
MFEMVectorL2Error::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params.addClassDescription(
      "Computes L2 error $\\left\\Vert \vec u_{ex} - \vec u_{h}\\right\\Vert_{\rm L2}$ for vector "
      "gridfucntions.");
  params.addParam<FunctionName>("function", "The analytic solution to compare against.");
  params.addParam<VariableName>(
      "variable", "Name of the vector variable of which to find the norm of the error.");
  return params;
}

MFEMVectorL2Error::MFEMVectorL2Error(const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    _var_name(getParam<VariableName>("variable")),
    _coeff_name(getParam<FunctionName>("function")),
    _vec_coeff(getVectorCoefficient(_coeff_name)),
    _var(getMFEMProblem().getProblemData().gridfunctions.GetRef(_var_name))
{
}

void
MFEMVectorL2Error::initialize()
{
}

void
MFEMVectorL2Error::execute()
{
}

PostprocessorValue
MFEMVectorL2Error::getValue() const
{
  return _var.ComputeL2Error(_vec_coeff);
}

#endif

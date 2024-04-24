#include "MFEMVariableDependentFunctionCoefficient.h"

registerMooseObject("PlatypusApp", MFEMVariableDependentFunctionCoefficient);

InputParameters
MFEMVariableDependentFunctionCoefficient::validParams()
{
  InputParameters params = MFEMCoefficient::validParams();
  params.addParam<FunctionName>(
      "function", 1, "The function acting on the MFEM variable to return the coefficient");

  params.addParam<std::string>("coupled_variable", "The MFEMVariable the coefficient depends on.");
  return params;
}

MFEMVariableDependentFunctionCoefficient::MFEMVariableDependentFunctionCoefficient(
    const InputParameters & parameters)
  : MFEMCoefficient(parameters),
    hephaestus::CoupledCoefficient(hephaestus::InputParameters(
        {{"CoupledVariableName", getParam<std::string>("coupled_variable")}})),
    _func(getFunction("function"))
{
}

double
MFEMVariableDependentFunctionCoefficient::Eval(mfem::ElementTransformation & trans,
                                               const mfem::IntegrationPoint & ip)
{
  auto gf_value{hephaestus::CoupledCoefficient::Eval(trans, ip)};
  return _func.value(gf_value);
};

MFEMVariableDependentFunctionCoefficient::~MFEMVariableDependentFunctionCoefficient() {}

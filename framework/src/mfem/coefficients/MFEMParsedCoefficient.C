#include "MFEMParsedCoefficient.h"

registerMooseObject("MooseApp", MFEMParsedCoefficient);

InputParameters
MFEMParsedCoefficient::validParams()
{
  InputParameters params = MFEMParsedCoefficientHelper::validParams();
  params += MFEMParsedCoefficientBase::validParams();
  params.addClassDescription("MFEM Parsed Function Coefficient.");
  return params;
}

MFEMParsedCoefficient::MFEMParsedCoefficient(const InputParameters & parameters)
  : MFEMParsedCoefficientHelper(parameters, VariableNameMappingMode::USE_MOOSE_NAMES),
    MFEMParsedCoefficientBase(parameters)
{
  // Build function and optimize
  functionParse(_function,
                _constant_names,
                _constant_expressions,
                getParam<std::vector<std::string>>("mfem_coefficient_names"),
                getParam<std::vector<std::string>>("mfem_gridfunction_names"));
}

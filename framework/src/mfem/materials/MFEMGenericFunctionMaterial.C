#ifdef MFEM_ENABLED

#include "MFEMGenericFunctionMaterial.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGenericFunctionMaterial);

InputParameters
MFEMGenericFunctionMaterial::validParams()
{
  InputParameters params = MFEMMaterial::validParams();
  params.addClassDescription("Declares material scalar properties based on names and functions "
                             "prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<FunctionName>>(
      "prop_values", "The corresponding names of functions associated with the named properties");

  return params;
}

MFEMGenericFunctionMaterial::MFEMGenericFunctionMaterial(const InputParameters & parameters)
  : MFEMMaterial(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<FunctionName>>("prop_values"))
{
  if (_prop_names.size() != _prop_values.size())
    paramError("prop_names", "Must match the size of prop_values");

  for (const auto i : index_range(_prop_names))
    _properties.declareScalar(
        _prop_names[i],
        subdomainsToStrings(_block_ids),
        getMFEMProblem().getProperties().getScalarPropertyPtr(_prop_values[i]));
}

MFEMGenericFunctionMaterial::~MFEMGenericFunctionMaterial() {}

#endif

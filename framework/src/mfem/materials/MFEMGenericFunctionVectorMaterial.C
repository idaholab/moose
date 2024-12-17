#ifdef MFEM_ENABLED

#include "MFEMGenericFunctionVectorMaterial.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGenericFunctionVectorMaterial);

InputParameters
MFEMGenericFunctionVectorMaterial::validParams()
{
  InputParameters params = MFEMMaterial::validParams();
  params.addClassDescription("Declares material vector properties based on names and functions "
                             "prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<FunctionName>>(
      "prop_values", "The corresponding names of functions associated with the named properties");

  return params;
}

MFEMGenericFunctionVectorMaterial::MFEMGenericFunctionVectorMaterial(
    const InputParameters & parameters)
  : MFEMMaterial(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<FunctionName>>("prop_values"))
{
  if (_prop_names.size() != _prop_values.size())
    paramError("prop_names", "Must match the size of prop_values");

  for (const auto i : index_range(_prop_names))
    _properties.declareVector(
        _prop_names[i],
        subdomainsToStrings(_block_ids),
        getMFEMProblem().getProperties().getVectorPropertyPtr(_prop_values[i]));
}

MFEMGenericFunctionVectorMaterial::~MFEMGenericFunctionVectorMaterial() {}

#endif

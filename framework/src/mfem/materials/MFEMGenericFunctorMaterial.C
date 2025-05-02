#ifdef MFEM_ENABLED

#include "MFEMGenericFunctorMaterial.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGenericFunctorMaterial);

InputParameters
MFEMGenericFunctorMaterial::validParams()
{
  InputParameters params = MFEMMaterial::validParams();
  params.addClassDescription("Declares material scalar properties based on names and coefficients "
                             "prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<MFEMScalarCoefficientName>>(
      "prop_values",
      "The corresponding names of coefficients associated with the named properties. A coefficient "
      "can be any "
      "of the following: a variable, an MFEM material property, a function, or a post-processor.");

  return params;
}

MFEMGenericFunctorMaterial::MFEMGenericFunctorMaterial(const InputParameters & parameters)
  : MFEMMaterial(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<MFEMScalarCoefficientName>>("prop_values"))
{
  if (_prop_names.size() != _prop_values.size())
    paramError("prop_names", "Must match the size of prop_values");

  for (const auto i : index_range(_prop_names))
    _properties.declareScalarProperty(
        _prop_names[i], subdomainsToStrings(_block_ids), _prop_values[i]);
}

MFEMGenericFunctorMaterial::~MFEMGenericFunctorMaterial() {}

#endif

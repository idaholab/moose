#ifdef MFEM_ENABLED

#include "MFEMGenericConstantMaterial.h"

registerMooseObject("MooseApp", MFEMGenericConstantMaterial);

InputParameters
MFEMGenericConstantMaterial::validParams()
{
  InputParameters params = MFEMMaterial::validParams();
  params.addClassDescription("Declares constant material properties based on names and values "
                             "prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<Real>>("prop_values",
                                             "The values associated with the named properties");

  params.declareControllable("prop_values");
  return params;
}

MFEMGenericConstantMaterial::MFEMGenericConstantMaterial(const InputParameters & parameters)
  : MFEMMaterial(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<Real>>("prop_values"))
{
  if (_prop_names.size() != _prop_values.size())
    paramError("prop_names", "Must match the number of prop_values");

  for (const auto i : index_range(_prop_names))
    _properties.declareScalarProperty<mfem::ConstantCoefficient>(
        _prop_names[i], subdomainsToStrings(_block_ids), _prop_values[i]);
}

MFEMGenericConstantMaterial::~MFEMGenericConstantMaterial() {}

#endif

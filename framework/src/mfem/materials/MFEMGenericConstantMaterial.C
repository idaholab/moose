#include "MFEMGenericConstantMaterial.h"

registerMooseObject("PlatypusApp", MFEMGenericConstantMaterial);

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
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_names != num_values)
    mooseError(
        "Number of prop_names must match the number of prop_values for a GenericConstantMaterial!");

  _num_props = num_names;
  for (unsigned int i = 0; i < _num_props; i++)
  {
    _properties.declareScalar<mfem::ConstantCoefficient>(
        _prop_names[i], subdomainsToStrings(_block_ids), _prop_values[i]);
  }
}

MFEMGenericConstantMaterial::~MFEMGenericConstantMaterial() {}

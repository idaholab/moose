#include "MFEMGenericFunctionMaterial.h"

registerMooseObject("PlatypusApp", MFEMGenericFunctionMaterial);

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
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_names != num_values)
    mooseError(
        "Number of prop_names must match the number of prop_values for a GenericConstantMaterial!");

  _num_props = num_names;
  for (unsigned int i = 0; i < _num_props; i++)
  {
    // FIXME: Ideally this would support arbitrary dimensions
    _properties.declareScalar(
        _prop_names[i],
        [&func = getFunctionByName(_prop_values[i])](const mfem::Vector & p,
                                                     double t) -> mfem::real_t
        { return func.value(t, pointFromMFEMVector(p)); },
        subdomainsToStrings(_block_ids));
  }
}

MFEMGenericFunctionMaterial::~MFEMGenericFunctionMaterial() {}

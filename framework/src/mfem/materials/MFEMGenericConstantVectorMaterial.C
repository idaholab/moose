#include "MFEMGenericConstantVectorMaterial.h"

registerMooseObject("MooseApp", MFEMGenericConstantVectorMaterial);

InputParameters
MFEMGenericConstantVectorMaterial::validParams()
{
  InputParameters params = MFEMMaterial::validParams();
  params.addClassDescription("Declares constant vector material properties based on names and "
                             "values prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<Real>>("prop_values",
                                             "The values associated with the named properties");

  params.declareControllable("prop_values");
  params.addParam<int>("dim", 3, "The dimension of the vector properties.");
  return params;
}

MFEMGenericConstantVectorMaterial::MFEMGenericConstantVectorMaterial(
    const InputParameters & parameters)
  : MFEMMaterial(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<Real>>("prop_values")),
    _prop_dims(getParam<int>("dim"))
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_names * _prop_dims != num_values)
    mooseError("Number of prop_values must be equal to dim * number of properties for a "
               "GenericConstantMaterial!");

  _num_props = num_names;
  for (unsigned int i = 0; i < _num_props; i++)
  {
    mfem::Vector vec(_prop_dims);
    for (int j = 0; j < _prop_dims; j++)
    {
      vec[j] = _prop_values[i * _prop_dims + j];
    }
    _properties.declareVector<mfem::VectorConstantCoefficient>(
        _prop_names[i], subdomainsToStrings(_block_ids), vec);
  }
}

MFEMGenericConstantVectorMaterial::~MFEMGenericConstantVectorMaterial() {}

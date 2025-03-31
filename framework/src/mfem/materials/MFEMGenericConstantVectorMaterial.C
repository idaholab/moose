#ifdef MFEM_ENABLED

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
  if (_prop_names.size() * _prop_dims != _prop_values.size())
    paramError("prop_values", "Number of values must be equal to dim * number of properties");

  for (const auto i : index_range(_prop_names))
  {
    mfem::Vector vec(_prop_dims);
    for (int j = 0; j < _prop_dims; j++)
      vec[j] = _prop_values[i * _prop_dims + j];
    _properties.declareVectorProperty<mfem::VectorConstantCoefficient>(
        _prop_names[i], subdomainsToStrings(_block_ids), vec);
  }
}

MFEMGenericConstantVectorMaterial::~MFEMGenericConstantVectorMaterial() {}

#endif

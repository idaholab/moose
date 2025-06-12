//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMGenericFunctorMaterial.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGenericFunctorMaterial);

InputParameters
MFEMGenericFunctorMaterial::validParams()
{
  InputParameters params = MFEMFunctorMaterial::validParams();
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
  : MFEMFunctorMaterial(parameters),
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

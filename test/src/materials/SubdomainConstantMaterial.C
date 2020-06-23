//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainConstantMaterial.h"

#include "MooseMesh.h"
#include "FEProblem.h"

registerMooseObject("MooseTestApp", SubdomainConstantMaterial);

InputParameters
SubdomainConstantMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<MaterialPropertyName>("mat_prop_name", "Material property name");
  params.addParam<std::vector<Real>>(
      "values",
      "Values of the material property on the subdomains (must be paired up with blocks)");

  params.set<MooseEnum>("constant_on") = "subdomain";
  params.suppressParameter<MooseEnum>("constant_on");
  return params;
}

SubdomainConstantMaterial::SubdomainConstantMaterial(const InputParameters & parameters)
  : Material(parameters),
    _mat_prop_name(getParam<MaterialPropertyName>("mat_prop_name")),
    _mat_prop(declareProperty<Real>(_mat_prop_name))
{
  auto & blocks = getParam<std::vector<SubdomainName>>("block");
  auto & values = getParam<std::vector<Real>>("values");
  auto vec_ids = _mesh.getSubdomainIDs(blocks);
  if (values.size() != vec_ids.size())
    mooseError("number of values is not equal to the number of blocks");

  for (unsigned int i = 0; i < vec_ids.size(); ++i)
    _mapped_values[vec_ids[i]] = values[i];
}

void
SubdomainConstantMaterial::computeQpProperties()
{
  auto it = _mapped_values.find(_current_subdomain_id);
  if (it != _mapped_values.end())
    _mat_prop[_qp] = it->second;
  else
    mooseError("material property ",
               _mat_prop_name,
               " is used on block ",
               _current_subdomain_id,
               " where it is not defined");
}

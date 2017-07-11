/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SubdomainConstantMaterial.h"

#include "MooseMesh.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<SubdomainConstantMaterial>()
{
  InputParameters params = validParams<Material>();
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
SubdomainConstantMaterial::computeSubdomainProperties()
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

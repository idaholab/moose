//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMBlockRestrictable.h"

InputParameters
MFEMBlockRestrictable::validParams()
{
  // Create InputParameters object that will be appended to the parameters for the inheriting object
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<SubdomainName>>(
      "block",
      {},
      "The list of subdomain subdomain_names (ids) that this "
      "object will be restricted to. Leave empty to apply "
      "to all subdomain_names.");
  return params;
}

MFEMBlockRestrictable::MFEMBlockRestrictable(const InputParameters & parameters,
                                             const mfem::ParMesh & mfem_mesh)
  : _mfem_mesh(mfem_mesh),
    _subdomain_names(parameters.get<std::vector<SubdomainName>>("block")),
    _subdomain_attributes(_subdomain_names.size())
{
  _subdomain_attributes = subdomainsToAttributes(_subdomain_names);
  mfem::common::AttrToMarker(
      _mfem_mesh.attributes.Max(), _subdomain_attributes, _subdomain_markers);
}

mfem::Array<int>
MFEMBlockRestrictable::subdomainsToAttributes(const std::vector<SubdomainName> & subdomain_names)
{
  mfem::Array<int> attributes(subdomain_names.size());
  auto & mesh = getMesh();
  std::transform(
      subdomain_names.begin(),
      subdomain_names.end(),
      attributes.begin(),
      [this, &mesh](const SubdomainName & subdomain) -> int
      {
        try
        {
          // Is this a block ID?
          return std::stoi(subdomain);
        }
        catch (...)
        {
          // It was not
          auto & subdomain_ids = mesh.attribute_sets.GetAttributeSet(subdomain);
          if (subdomain_ids.Size() != 1)
            mooseError(
                "There should be a 1-to-1 correspondence between subdomain name and subdomain ID");
          return subdomain_ids[0];
        }
      });
  return attributes;
}

std::vector<std::string>
MFEMBlockRestrictable::subdomainsToStrings(const std::vector<SubdomainName> & subdomain_names)
{
  auto attributes = subdomainsToAttributes(subdomain_names);
  std::vector<std::string> subdomain_attr_strings(subdomain_names.size());
  for (const auto i : index_range(subdomain_names))
  {
    subdomain_attr_strings[i] = std::to_string(attributes[i]);
  }
  return subdomain_attr_strings;
}

#endif

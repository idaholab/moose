//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMBlockRestrictable.h"

InputParameters
MFEMBlockRestrictable::validParams()
{
  // Create InputParameters object that will be appended to the parameters for the inheriting object
  InputParameters params = emptyInputParameters();
  // Create user-facing 'boundary' input for restricting inheriting object to boundaries
  // MFEM uses the boundary -1 to signify every sideset
  params.addParam<std::vector<SubdomainName>>("block",
                                              {},
                                              "The list of subdomains (names or ids) that this "
                                              "object will be restricted to. Leave empty to apply "
                                              "to all subdomains.");
  return params;
}

MFEMBlockRestrictable::MFEMBlockRestrictable(const InputParameters & parameters,
                                             const mfem::ParMesh & mfem_mesh)
  : _mfem_mesh(mfem_mesh),
    _subdomain_names(parameters.get<std::vector<SubdomainName>>("block")),
    _subdomain_attributes(_subdomain_names.size())
{
  _subdomain_attributes = subdomainsToAttributes(_subdomain_names);
  if (!_subdomain_attributes.IsEmpty())
    mfem::common::AttrToMarker(
        _mfem_mesh.attributes.Max(), _subdomain_attributes, _subdomain_markers);
}

mfem::Array<int>
MFEMBlockRestrictable::subdomainsToAttributes(const std::vector<SubdomainName> & subdomain_names)
{
  mfem::Array<int> attributes;
  auto & mesh = getMesh();

  for (const SubdomainName & subdomain_name : subdomain_names)
  {
    try
    {
      // Is this a block ID?
      const int attribute_id = std::stoi(subdomain_name);
      attributes.Append(attribute_id);
    }
    catch (...)
    {
      // It was not
      auto & subdomain_ids = mesh.attribute_sets.GetAttributeSet(subdomain_name);
      for (const auto & subdomain_id : subdomain_ids)
        attributes.Append(subdomain_id);
    }
  }
  return attributes;
}

std::vector<std::string>
MFEMBlockRestrictable::subdomainsToStrings(const std::vector<SubdomainName> & subdomain_names)
{
  auto attributes = subdomainsToAttributes(subdomain_names);
  std::vector<std::string> subdomain_attr_strings(subdomain_names.size());
  for (const auto i : index_range(subdomain_names))
    subdomain_attr_strings[i] = std::to_string(attributes[i]);
  return subdomain_attr_strings;
}

#endif

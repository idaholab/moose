//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMBoundaryRestrictable.h"

InputParameters
MFEMBoundaryRestrictable::validParams()
{
  // Create InputParameters object that will be appended to the parameters for the inheriting object
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<BoundaryName>>(
      "boundary",
      {},
      "The list of boundaries (names or ids) that this object will be restricted to. "
      "Leave empty to apply to all boundaries.");
  return params;
}

MFEMBoundaryRestrictable::MFEMBoundaryRestrictable(const InputParameters & parameters,
                                                   const mfem::ParMesh & mfem_mesh)
  : _mfem_mesh(mfem_mesh),
    _boundary_names(parameters.get<std::vector<BoundaryName>>("boundary")),
    _boundary_attributes(boundariesToAttributes(parameters)),
    _boundary_markers(_mfem_mesh.bdr_attributes.IsEmpty()
                          ? mfem::Array<int>()
                          : mfem::AttributeSets::AttrToMarker(_mfem_mesh.bdr_attributes.Max(),
                                                              _boundary_attributes.IsEmpty()
                                                                  ? _mfem_mesh.bdr_attributes
                                                                  : _boundary_attributes))
{
}

mfem::Array<int>
MFEMBoundaryRestrictable::boundariesToAttributes(const InputParameters & parameters)
{
  mfem::Array<int> attributes;
  auto & mesh = getMesh();

  for (const BoundaryName & boundary_name : _boundary_names)
  {
    try
    {
      // Is this a boundary ID?
      const int attribute_id = std::stoi(boundary_name);
      attributes.Append(attribute_id);
    }
    catch (...)
    {
      // It was not
      if (!mesh.bdr_attribute_sets.AttributeSetExists(boundary_name))
        parameters.paramError("boundary", "the mesh has no boundary named '", boundary_name, "'.");
      auto & subdomain_ids = mesh.bdr_attribute_sets.GetAttributeSet(boundary_name);
      for (const auto & subdomain_id : subdomain_ids)
        attributes.Append(subdomain_id);
    }
  }

  for (const auto attribute : attributes)
    if (mesh.bdr_attributes.Find(attribute) == -1)
      parameters.paramError("boundary", "the mesh has no boundary with ID ", attribute, ".");
  return attributes;
}

std::vector<std::string>
MFEMBoundaryRestrictable::boundariesToStrings()
{
  const auto & attrs = _boundary_attributes;
  std::vector<std::string> strs(attrs.Size());
  std::transform(attrs.begin(), attrs.end(), strs.begin(), [](int n) { return std::to_string(n); });
  return strs;
}

#endif

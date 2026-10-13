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
      {"-1"},
      "The list of boundaries (ids or names) from the mesh where this object applies. "
      "Defaults to all boundaries.");
  return params;
}

MFEMBoundaryRestrictable::MFEMBoundaryRestrictable(const InputParameters & parameters,
                                                   const mfem::ParMesh & mfem_mesh)
  : _mfem_mesh(mfem_mesh),
    _boundary_names(parameters.get<std::vector<BoundaryName>>("boundary")),
    _boundary_attributes(boundariesToAttributes())
{
  if (!_boundary_attributes.IsEmpty())
    mfem::common::AttrToMarker(
        _mfem_mesh.bdr_attributes.Max(), _boundary_attributes, _boundary_markers);
}

mfem::Array<int>
MFEMBoundaryRestrictable::boundariesToAttributes()
{
  mfem::Array<int> attributes;
  auto & mesh = getMesh();

  for (const BoundaryName & boundary_name : _boundary_names)
  {
    try
    {
      // Is this a sideset ID?
      const int attribute_id = std::stoi(boundary_name);
      attributes.Append(attribute_id);
    }
    catch (...)
    {
      // It was not
      auto & boundary_ids = mesh.bdr_attribute_sets.GetAttributeSet(boundary_name);
      for (const auto & boundary_id : boundary_ids)
        attributes.Append(boundary_id);
    }
  }
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

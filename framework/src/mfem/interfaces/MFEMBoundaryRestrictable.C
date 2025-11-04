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
    _boundary_attributes(_boundary_names.size())
{
  _boundary_attributes = boundariesToAttributes(_boundary_names);
  if (!_boundary_attributes.IsEmpty())
    mfem::common::AttrToMarker(
        _mfem_mesh.bdr_attributes.Max(), _boundary_attributes, _boundary_markers);
}

mfem::Array<int>
MFEMBoundaryRestrictable::boundariesToAttributes(const std::vector<BoundaryName> & boundary_names)
{
  mfem::Array<int> attributes(boundary_names.size());
  auto & mesh = getMesh();
  std::transform(
      boundary_names.begin(),
      boundary_names.end(),
      attributes.begin(),
      [&mesh](const BoundaryName & boundary) -> int
      {
        try
        {
          // Is this a sideset ID?
          return std::stoi(boundary);
        }
        catch (...)
        {
          // It was not
          auto & boundary_ids = mesh.bdr_attribute_sets.GetAttributeSet(boundary);
          if (boundary_ids.Size() != 1)
            mooseError(
                "There should be a 1-to-1 correspondence between boundary name and boundary ID");
          return boundary_ids[0];
        }
      });
  return attributes;
}

#endif

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMBoundaryCondition.h"
#include "MFEMProblem.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"

InputParameters
MFEMBoundaryCondition::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.addClassDescription("Base class for applying boundary conditions to MFEM problems.");
  params.registerBase("BoundaryCondition");
  // Create user-facing 'boundary' input for restricting inheriting object to boundaries
  // MFEM uses the boundary -1 to signify every sideset
  params.addParam<std::vector<BoundaryName>>(
      "boundary",
      {"-1"},
      "The list of boundaries (ids) from the mesh where this boundary condition applies. "
      "Defaults to applying BC on all boundaries.");
  params.addParam<VariableName>("variable", "Variable on which to apply the boundary condition");
  return params;
}

MFEMBoundaryCondition::MFEMBoundaryCondition(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _test_var_name(getParam<VariableName>("variable")),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary")),
    _bdr_attributes(_boundary_names.size())
{
  mfem::ParMesh & mesh(*getMFEMProblem()
                            .getProblemData()
                            .gridfunctions.GetRef(_test_var_name)
                            .ParFESpace()
                            ->GetParMesh());

  for (const auto i : index_range(_boundary_names))
  {
    const auto & boundary_name = _boundary_names[i];
    try
    {
      // Is this a boundary ID
      _bdr_attributes[i] = std::stoi(boundary_name);
    }
    catch (...)
    {
      // It was not
      auto & bnd_ids = mesh.bdr_attribute_sets.GetAttributeSet(boundary_name);
      if (bnd_ids.Size() != 1)
        this->mooseError(
            "There should be a 1-to-1 correspondence between boundary name and boundary ID");
      _bdr_attributes[i] = bnd_ids[0];
    }
  }
  mfem::common::AttrToMarker(mesh.bdr_attributes.Max(), _bdr_attributes, _bdr_markers);
}

#endif

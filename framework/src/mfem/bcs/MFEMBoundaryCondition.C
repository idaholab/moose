#ifdef MFEM_ENABLED

#include "MFEMBoundaryCondition.h"
#include "MFEMProblem.h"
#include "MFEMMesh.h"
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
  params.addParam<std::vector<BoundaryName>>(
      "boundary",
      "The list of boundaries (ids or names) from the mesh where this boundary condition applies");
  params.addParam<std::string>("variable", "Variable on which to apply the boundary condition");
  return params;
}

MFEMBoundaryCondition::MFEMBoundaryCondition(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _test_var_name(getParam<std::string>("variable")),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary")),
    _bdr_attributes(_boundary_names.size())
{
  auto & mesh = getMFEMProblem().mesh().getMFEMParMesh();

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
}

mfem::Array<int>
MFEMBoundaryCondition::GetMarkers(mfem::Mesh & mesh)
{
  mfem::common::AttrToMarker(mesh.bdr_attributes.Max(), _bdr_attributes, _bdr_markers);
  return _bdr_markers;
}

#endif

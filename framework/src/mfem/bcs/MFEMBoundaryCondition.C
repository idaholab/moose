#include "MFEMBoundaryCondition.h"
#include "mfem/miniapps/common/mesh_extras.hpp"

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
  for (unsigned int i = 0; i < _boundary_names.size(); ++i)
  {
    _bdr_attributes[i] = std::stoi(_boundary_names[i]);
  }
}

mfem::Array<int>
MFEMBoundaryCondition::GetMarkers(mfem::Mesh & mesh)
{
  mfem::common::AttrToMarker(mesh.bdr_attributes.Max(), _bdr_attributes, _bdr_markers);
  return _bdr_markers;
}

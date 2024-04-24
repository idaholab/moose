#include "MFEMBoundaryCondition.h"

registerMooseObject("PlatypusApp", MFEMBoundaryCondition);

InputParameters
MFEMBoundaryCondition::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.registerBase("BoundaryCondition");

  // Create user-facing 'boundary' input for restricting inheriting object to boundaries
  params.addParam<std::vector<BoundaryName>>(
      "boundary",
      "The list of boundaries (ids or names) from the mesh where this boundary condition applies");
  params.addParam<std::string>("variable", "Variable on which to apply the boundary condition");
  return params;
}

MFEMBoundaryCondition::MFEMBoundaryCondition(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary")),
    bdr_attr(_boundary_names.size())
{
  for (unsigned int i = 0; i < _boundary_names.size(); ++i)
  {
    bdr_attr[i] = std::stoi(_boundary_names[i]);
  }
  _boundary_condition =
      std::make_shared<hephaestus::BoundaryCondition>(getParam<std::string>("variable"), bdr_attr);
}

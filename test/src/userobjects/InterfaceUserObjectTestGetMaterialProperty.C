//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceUserObjectTestGetMaterialProperty.h"
#include "MooseMesh.h"
registerMooseObject("MooseTestApp", InterfaceUserObjectTestGetMaterialProperty);

defineLegacyParams(InterfaceUserObjectTestGetMaterialProperty);

InputParameters
InterfaceUserObjectTestGetMaterialProperty::validParams()
{
  InputParameters params = InterfaceUserObject::validParams();
  params.addRequiredParam<MaterialPropertyName>("property", "The property name");
  params.addRequiredParam<MaterialPropertyName>("property_neighbor", "The neighbor property name");
  params.addRequiredParam<MaterialPropertyName>("property_boundary", "The neighbor property name");
  params.addRequiredParam<MaterialPropertyName>("property_interface",
                                                "The interface property name");
  params.addClassDescription(
      "This userobject tests the capabilities of the interface user object to"
      "get synchornised values of bulk, boundary and interface material properties ");
  return params;
}

InterfaceUserObjectTestGetMaterialProperty::InterfaceUserObjectTestGetMaterialProperty(
    const InputParameters & parameters)
  : InterfaceUserObject(parameters),
    _mp(getMaterialProperty<Real>("property")),
    _mp_neighbor(getNeighborMaterialProperty<Real>("property_neighbor")),
    _mp_boundary(getMaterialProperty<Real>("property_boundary")),
    _mp_interface(getMaterialProperty<Real>("property_interface"))
{
}

InterfaceUserObjectTestGetMaterialProperty::~InterfaceUserObjectTestGetMaterialProperty() {}

void
InterfaceUserObjectTestGetMaterialProperty::initialSetup()
{
  // define the boundary map and retrieve element side and boundary_ID
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> elem_side_bid =
      _mesh.buildSideList();

  // retrieve on which boundary this UO operates
  std::set<BoundaryID> boundaryList = boundaryIDs();

  // clear map values
  _map_values.clear();

  // initialize the map_values looping over all the element and sides
  for (unsigned int i = 0; i < elem_side_bid.size(); i++)
  {
    // check if this element side is part of the boundary, if so add element side to the interface
    // map
    if (boundaryList.find(std::get<2>(elem_side_bid[i])) != boundaryList.end())
    {
      // make pair
      std::pair<dof_id_type, unsigned int> elem_side_pair =
          std::make_pair(std::get<0>(elem_side_bid[i]), std::get<1>(elem_side_bid[i]));
      // initialize map elemenet
      std::vector<Real> var_values(0, 0);
      // add entry to the value map
      _map_values[elem_side_pair] = var_values;
    }
  }
}

void
InterfaceUserObjectTestGetMaterialProperty::execute()
{
  // find the entry on the map
  auto it = _map_values.find(std::make_pair(_current_elem->id(), _current_side));
  if (it != _map_values.end())
  {
    // insert two vector value for each qp
    auto & vec = _map_values[std::make_pair(_current_elem->id(), _current_side)];
    vec.resize(_qrule->n_points());

    // loop over qps and do stuff
    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    {
      // check material property values

      mooseAssert(
          _mp[qp] ==
              1 * (_t_step + _fe_problem.nNonlinearIterations() * _fe_problem.nLinearIterations()),
          "InterfaceUserObjectTestGetMaterialProperty bad material property value"
              << _mp[qp] << " instead of "
              << (_t_step + _fe_problem.nNonlinearIterations() * _fe_problem.nLinearIterations()));
      mooseAssert(
          _mp_neighbor[qp] ==
              2 * (_t_step + _fe_problem.nNonlinearIterations() * _fe_problem.nLinearIterations()),
          "InterfaceUserObjectTestGetMaterialProperty bad neighbor material property value: "
              << _mp_neighbor[qp] << " instead of "
              << 2 * (_t_step +
                      _fe_problem.nNonlinearIterations() * _fe_problem.nLinearIterations()));
      mooseAssert(
          _mp_interface[qp] ==
              4 * (_t_step + _fe_problem.nNonlinearIterations() * _fe_problem.nLinearIterations()),
          "InterfaceUserObjectTestGetMaterialProperty bad interface material property value: "
              << _mp_interface[qp] << " instead of "
              << 4 * (_t_step +
                      _fe_problem.nNonlinearIterations() * _fe_problem.nLinearIterations()));
      mooseAssert(
          _mp_boundary[qp] ==
              3 * (_t_step + _fe_problem.nNonlinearIterations() * _fe_problem.nLinearIterations()),
          "InterfaceUserObjectTestGetMaterialProperty bad boundary material property value: "
              << _mp_boundary[qp] << " instead of "
              << 3 * (_t_step +
                      _fe_problem.nNonlinearIterations() * _fe_problem.nLinearIterations()));
    }
  }
  else
    mooseError(
        "InterfaceUserObjectTestGetMaterialProperty:: cannot find the required element and side");
}

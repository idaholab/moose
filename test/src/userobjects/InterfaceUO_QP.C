//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceUO_QP.h"
#include "MooseMesh.h"
registerMooseObject("MooseTestApp", InterfaceUO_QP);

template <>
InputParameters
validParams<InterfaceUO_QP>()
{
  InputParameters params = validParams<InterfaceUserObject>();
  params.addParam<MaterialPropertyName>("diffusivity",
                                        0.0,
                                        "The name of the diffusivity material property that "
                                        "will be used in the flux computation.");
  params.addParam<bool>(
      "use_old_prop",
      false,
      "A Boolean to indicate whether the current or old value of a material prop should be used.");
  params.addRequiredCoupledVar("variable", "the variable name");
  return params;
}

InterfaceUO_QP::InterfaceUO_QP(const InputParameters & parameters)
  : InterfaceUserObject(parameters),
    _u(coupledValue("variable")),
    _u_neighbor(coupledNeighborValue("variable")),
    _diffusivity_prop(getParam<bool>("use_old_prop") ? getMaterialPropertyOld<Real>("diffusivity")
                                                     : getMaterialProperty<Real>("diffusivity")),
    _neighbor_diffusivity_prop(getParam<bool>("use_old_prop")
                                   ? getNeighborMaterialPropertyOld<Real>("diffusivity")
                                   : getNeighborMaterialProperty<Real>("diffusivity"))
{
}

InterfaceUO_QP::~InterfaceUO_QP() {}

void
InterfaceUO_QP::initialize()
{

  // define the boundary map nad retrieve element side and boundary_ID
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> elem_side_bid =
      _mesh.buildSideList();

  // retrieve on which boudnary this UO operates
  std::set<BoundaryID> boundaryList = boundaryIDs();

  // clear map values
  _map_values.clear();

  // initialize the map_values looping over all the element and sides
  for (unsigned int i = 0; i < elem_side_bid.size(); i++)
  {
    // check if this boundary
    // if this element side is part of the boundary then add elements to the map
    if (boundaryList.find(std::get<2>(elem_side_bid[i])) != boundaryList.end())
    {

      // make pair
      std::pair<dof_id_type, unsigned int> elem_side_pair =
          std::make_pair(std::get<0>(elem_side_bid[i]), std::get<1>(elem_side_bid[i]));
      // initialize map elemenet
      std::vector<std::vector<Real>> var_values(0, std::vector<Real>(3, 0));

      // add entry to the value map
      _map_values[elem_side_pair] = var_values;
    }
  }
}

void
InterfaceUO_QP::execute()
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
      vec[qp].resize(3, 0);

      // compute average material property
      vec[qp][0] = (_diffusivity_prop[qp] + _neighbor_diffusivity_prop[qp]) / 2.0;
      // compute variable jump
      vec[qp][1] = _u[qp] - _u_neighbor[qp];
      vec[qp][2] = vec[qp][0] * (vec[qp][1] + 2);
    }
  }
  else
    mooseError("InterfaceUO_QP:: cannot fine the required element and side");
}

Real
InterfaceUO_QP::getMeanMatProp(dof_id_type elem, unsigned int side, unsigned int qp) const
{
  auto data = _map_values.find(std::make_pair(elem, side));
  if (data != _map_values.end())
    return data->second[qp][0];
  else
    mooseError("getMeanMatProp: can't find the given qp");
}

Real
InterfaceUO_QP::getVarJump(dof_id_type elem, unsigned int side, unsigned int qp) const
{
  auto data = _map_values.find(std::make_pair(elem, side));
  if (data != _map_values.end())
    return data->second[qp][1];
  else
    mooseError("getVarJump: can't find the given qp");
}

Real
InterfaceUO_QP::getNewBoundaryPropertyValue(dof_id_type elem,
                                            unsigned int side,
                                            unsigned int qp) const
{
  auto data = _map_values.find(std::make_pair(elem, side));
  if (data != _map_values.end())
    return data->second[qp][2];
  else
    mooseError("getNewBoundaryPropertyValue: can't find the given qp");
}

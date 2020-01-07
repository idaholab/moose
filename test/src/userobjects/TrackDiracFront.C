//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TrackDiracFront.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", TrackDiracFront);

InputParameters
TrackDiracFront::validParams()
{
  InputParameters params = NodalUserObject::validParams();

  params.addRequiredCoupledVar(
      "var", "Wherever this variable is close to 0.5 a Dirac point will be generated");

  return params;
}

TrackDiracFront::TrackDiracFront(const InputParameters & parameters)
  : NodalUserObject(parameters), _var_value(coupledValue("var"))
{
}

void
TrackDiracFront::initialize()
{
  _dirac_points.clear();
}

void
TrackDiracFront::execute()
{
  // Is the value near 0.5?
  if (_var_value[_qp] > 0.4 && _var_value[_qp] < 0.6)
  {
    Elem * elem = localElementConnectedToCurrentNode();
    _dirac_points.push_back(std::make_pair(elem, Point(*_current_node)));
  }
}

void
TrackDiracFront::threadJoin(const UserObject & y)
{
  const TrackDiracFront & tdf = static_cast<const TrackDiracFront &>(y);

  // Merge in the values from "y"
  _dirac_points.insert(_dirac_points.end(), tdf._dirac_points.begin(), tdf._dirac_points.end());
}

void
TrackDiracFront::finalize()
{
  // Nothing to do because we were careful to only record _local_ information - so no MPI
  // communication is necessary
}

Elem *
TrackDiracFront::localElementConnectedToCurrentNode()
{
  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map = _mesh.nodeToElemMap();
  auto node_to_elem_pair = node_to_elem_map.find(_current_node->id());
  mooseAssert(node_to_elem_pair != node_to_elem_map.end(), "Node missing in node to elem map");
  const std::vector<dof_id_type> & connected_elems = node_to_elem_pair->second;

  auto pid = processor_id(); // This processor id

  // Look through all of the elements connected to this node and find one owned by the local
  // processor
  for (auto elem_id : connected_elems)
  {
    Elem * elem = _mesh.elemPtr(elem_id);
    mooseAssert(elem, "Elem pointer is NULL");

    if (elem->processor_id() == pid) // Is this element owned by the local processor?
      return elem;
  }

  mooseError("Unable to locate a local element connected to this node!");
}

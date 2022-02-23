//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunctionBase.h"

InputParameters
VolumeJunctionBase::validParams()
{
  InputParameters params = FlowJunction::validParams();

  params.addRequiredParam<Real>("volume", "Volume of the junction [m^3]");
  params.addRequiredParam<Point>("position", "Spatial position of the center of the junction [m]");

  return params;
}

VolumeJunctionBase::VolumeJunctionBase(const InputParameters & params)
  : FlowJunction(params), _volume(getParam<Real>("volume")), _position(getParam<Point>("position"))
{
}

void
VolumeJunctionBase::setupMesh()
{
  FlowJunction::setupMesh();

  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem = _mesh.nodeToElemMap();
  for (auto & nid : _nodes)
  {
    const auto & it = node_to_elem.find(nid);
    if (it == node_to_elem.end())
      mooseError(name(), ": failed to find node ", nid, "in the mesh!");

    const std::vector<dof_id_type> & elems = it->second;
    for (const auto & e : elems)
      _connected_elems.push_back(e);
  }

}

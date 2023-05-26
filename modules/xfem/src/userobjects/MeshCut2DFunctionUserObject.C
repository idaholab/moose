//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCut2DFunctionUserObject.h"

#include "libmesh/string_to_enum.h"
#include "MooseMesh.h"
#include "MooseEnum.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/mesh_tools.h"
#include "Function.h"

registerMooseObject("XFEMApp", MeshCut2DFunctionUserObject);

InputParameters
MeshCut2DFunctionUserObject::validParams()
{
  InputParameters params = MeshCut2DUserObjectBase::validParams();
  params.addClassDescription("Creates a UserObject for a mesh cutter in 2D problems where crack "
                             "growth is specified by functions.");
  params.addRequiredParam<FunctionName>("growth_direction_x",
                                        "Function defining x-component of crack growth direction.");
  params.addRequiredParam<FunctionName>("growth_direction_y",
                                        "Function defining y-component of crack growth direction.");
  params.addRequiredParam<FunctionName>("growth_rate", "Function defining crack growth rate.");
  return params;
}

MeshCut2DFunctionUserObject::MeshCut2DFunctionUserObject(const InputParameters & parameters)
  : MeshCut2DUserObjectBase(parameters),
    _func_x(&getFunction("growth_direction_x")),
    _func_y(&getFunction("growth_direction_y")),
    _growth_function(&getFunction("growth_rate")),
    _time_of_previous_call_to_UO(std::numeric_limits<Real>::lowest())
{
}

void
MeshCut2DFunctionUserObject::initialize()
{
  // Only call crack growth function if time changed.
  if (_t > _time_of_previous_call_to_UO)
  {
    findActiveBoundaryGrowth();
    growFront();
  }
  _time_of_previous_call_to_UO = _t;
}

void
MeshCut2DFunctionUserObject::findActiveBoundaryGrowth()
{
  _active_front_node_growth_vectors.clear();
  Point zero; // Used for checking whether direction is zero
  for (unsigned int i = 0; i < _original_and_current_front_node_ids.size(); ++i)
  {
    // compute growth direction
    Point dir;
    Node * this_node = _cutter_mesh->node_ptr(_original_and_current_front_node_ids[i].second);
    mooseAssert(this_node, "Node is NULL");
    Point & this_point = *this_node;
    dir(0) = _func_x->value(_t, this_point);
    dir(1) = _func_y->value(_t, this_point);
    dir = dir / dir.norm();

    // compute growth amount/velocity
    Point nodal_offset;
    Real velo = _growth_function->value(_t, Point(0, 0, 0));
    nodal_offset = dir * velo * _dt;
    // only increment the crack if the growth is positive
    if (!nodal_offset.absolute_fuzzy_equals(zero))
      _active_front_node_growth_vectors.push_back(
          std::make_pair(_original_and_current_front_node_ids[i].second, nodal_offset));
  }
}

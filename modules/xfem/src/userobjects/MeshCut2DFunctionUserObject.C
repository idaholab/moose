//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCut2DFunctionUserObject.h"

#include "MooseError.h"
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
  params.addRequiredParam<FunctionName>("function_x", "Growth function for x direction");
  params.addRequiredParam<FunctionName>("function_y", "Growth function for y direction");
  params.addRequiredParam<FunctionName>("function_v", "Growth speed function");
  return params;
}

MeshCut2DFunctionUserObject::MeshCut2DFunctionUserObject(const InputParameters & parameters)
  : MeshCut2DUserObjectBase(parameters),
    _func_x(&getFunction("function_x")),
    _func_y(&getFunction("function_y")),
    _func_v(&getFunction("function_v"))
{
}

void
MeshCut2DFunctionUserObject::initialize()
{
  // following logic only calls crack growth function if time changed.
  // This deals with max_xfem_update > 1.  Error if the timestep drops below this
  if ((_t - _time_of_previous_call_to_UO) > libMesh::TOLERANCE)
  {
    findActiveBoundaryGrowth();
    growFront();
  }
  else if (_dt < libMesh::TOLERANCE && _t_step != 0)
    mooseError("timestep size must be greater than " + Moose::stringify(libMesh::TOLERANCE) +
               ", for crack to grow. dt=" + Moose::stringify(_dt));

  _time_of_previous_call_to_UO = _t;
}

void
MeshCut2DFunctionUserObject::findActiveBoundaryGrowth()
{
  _active_front_node_growth_vectors.clear();
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
      Real velo = _func_v->value(_t, Point(0, 0, 0));
      nodal_offset = dir * velo * _dt;

      _active_front_node_growth_vectors.push_back(std::make_pair(_original_and_current_front_node_ids[i].second,
                                                  nodal_offset));
  }
}

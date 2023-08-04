//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapHeatPointSourceMaster.h"
#include "SystemBase.h"
#include "PenetrationInfo.h"
#include "MooseMesh.h"

#include "libmesh/string_to_enum.h"

registerMooseObject("HeatConductionApp", GapHeatPointSourceMaster);

InputParameters
GapHeatPointSourceMaster::validParams()
{
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = DiracKernel::validParams();
  params.addParam<BoundaryName>("boundary", "The primary boundary");
  params.addParam<BoundaryName>("secondary", "The secondary boundary");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method",
                               "Method to use to smooth normals (edge_based|nodal_normal_based)");

  return params;
}

GapHeatPointSourceMaster::GapHeatPointSourceMaster(const InputParameters & parameters)
  : DiracKernel(parameters),
    _penetration_locator(
        getPenetrationLocator(getParam<BoundaryName>("boundary"),
                              getParam<BoundaryName>("secondary"),
                              Utility::string_to_enum<Order>(getParam<MooseEnum>("order")))),
    _secondary_flux(_sys.getVector("secondary_flux"))
{
  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(
        parameters.get<std::string>("normal_smoothing_method"));
}

void
GapHeatPointSourceMaster::addPoints()
{
  point_to_node.clear();
  _secondary_flux.close();

  for (const auto & [id, pinfo] : _penetration_locator._penetration_info)
  {
    if (!pinfo)
      continue;

    const auto & node = _mesh.nodeRef(id);
    // Skip this pinfo if there are no DOFs on this node.
    if (node.n_comp(_sys.number(), _var.number()) < 1)
      continue;

    addPoint(pinfo->_elem, pinfo->_closest_point);
    point_to_node[pinfo->_closest_point] = &node;
  }
}

Real
GapHeatPointSourceMaster::computeQpResidual()
{
  const Node & node = *point_to_node[_current_point];
  const auto dof_number = node.dof_number(_sys.number(), _var.number(), 0);

  return -_test[_i][_qp] * _secondary_flux(dof_number);
}

Real
GapHeatPointSourceMaster::computeQpJacobian()
{
  return 0.0;
}

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

template <>
InputParameters
validParams<GapHeatPointSourceMaster>()
{
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<BoundaryName>("boundary", "The master boundary");
  params.addRequiredParam<BoundaryName>("slave", "The slave boundary");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");
  MooseEnum smoothing_method("edge_based nodal_normal_based");
  params.addParam<MooseEnum>(
      "normal_smoothing_method", smoothing_method, "Method to use to smooth normals");
  params.addParam<UserObjectName>("normal_normals",
                                  "The name of the user object that provides the nodal normals.");

  return params;
}

GapHeatPointSourceMaster::GapHeatPointSourceMaster(const InputParameters & parameters)
  : DiracKernel(parameters),
    _penetration_locator(
        getPenetrationLocator(getParam<BoundaryName>("boundary"),
                              getParam<BoundaryName>("slave"),
                              Utility::string_to_enum<Order>(getParam<MooseEnum>("order")))),
    _slave_flux(_sys.getVector("slave_flux"))
{
  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (isParamValid("normal_smoothing_method"))
  {
    MooseEnum smoothing_method = getParam<MooseEnum>("normal_smoothing_method");
    if (smoothing_method == "edge_based")
    {
      if (!isParamValid("normal_smoothing_distance"))
        mooseError(
            name(),
            ": For edge based normal smoothing, normal_smoothing_distance parameter must be set.");
      _penetration_locator.setEdgeBaseSmoothingMethod(getParam<Real>("normal_smoothing_distance"));
    }
    else if (smoothing_method == "nodal_normal_based")
    {
      if (!isParamValid("nodal_normals"))
        mooseError(name(),
                   ": For nodal nodal based smoothing, nodal_normals parameter must be set.");
      _penetration_locator.setNodalNormalSmoothingMethod(getParam<UserObjectName>("nodal_normals"));
    }
    else
      mooseError(name(), ": Unknown smoothing method.");
  }
}

void
GapHeatPointSourceMaster::addPoints()
{
  point_to_info.clear();
  _slave_flux.close();

  std::map<dof_id_type, PenetrationInfo *>::iterator
      it = _penetration_locator._penetration_info.begin(),
      end = _penetration_locator._penetration_info.end();

  for (; it != end; ++it)
  {
    PenetrationInfo * pinfo = it->second;

    // Skip this pinfo if there are no DOFs on this node.
    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _var.number()) < 1)
      continue;

    addPoint(pinfo->_elem, pinfo->_closest_point);
    point_to_info[pinfo->_closest_point] = pinfo;
  }
}

Real
GapHeatPointSourceMaster::computeQpResidual()
{
  PenetrationInfo * pinfo = point_to_info[_current_point];
  const Node * node = pinfo->_node;
  long int dof_number = node->dof_number(_sys.number(), _var.number(), 0);

  return -_test[_i][_qp] * _slave_flux(dof_number);
}

Real
GapHeatPointSourceMaster::computeQpJacobian()
{
  return 0.0;
}

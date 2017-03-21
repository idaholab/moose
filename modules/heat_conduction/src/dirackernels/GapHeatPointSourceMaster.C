/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
  params.addParam<std::string>("normal_smoothing_method",
                               "Method to use to smooth normals (edge_based|nodal_normal_based)");

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

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(
        parameters.get<std::string>("normal_smoothing_method"));
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

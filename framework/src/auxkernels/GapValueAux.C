//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapValueAux.h"

#include "MooseMesh.h"
#include "SystemBase.h"
#include "MooseEnum.h"
#include "PenetrationLocator.h"

#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<GapValueAux>()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<AuxKernel>();
  params.set<bool>("_dual_restrictable") = true;
  params.addRequiredParam<BoundaryName>("paired_boundary",
                                        "The boundary on the other side of a gap.");
  params.addRequiredParam<VariableName>("paired_variable", "The variable to get the value of.");
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
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>(
      "warnings", false, "Whether to output warning messages concerning nodes not being found");
  return params;
}

GapValueAux::GapValueAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _penetration_locator(
        _nodal ? getPenetrationLocator(
                     parameters.get<BoundaryName>("paired_boundary"),
                     boundaryNames()[0],
                     Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))
               : getQuadraturePenetrationLocator(
                     parameters.get<BoundaryName>("paired_boundary"),
                     boundaryNames()[0],
                     Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))),
    _moose_var(_subproblem.getVariable(_tid, getParam<VariableName>("paired_variable"))),
    _serialized_solution(_moose_var.sys().currentSolution()),
    _dof_map(_moose_var.dofMap()),
    _warnings(getParam<bool>("warnings"))
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

  Order pairedVarOrder(_moose_var.order());
  Order gvaOrder(Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")));
  if (pairedVarOrder != gvaOrder && pairedVarOrder != CONSTANT)
    mooseError("ERROR: specified order for GapValueAux (",
               Utility::enum_to_string<Order>(gvaOrder),
               ") does not match order for paired_variable \"",
               _moose_var.name(),
               "\" (",
               Utility::enum_to_string<Order>(pairedVarOrder),
               ")");
}

Real
GapValueAux::computeValue()
{
  const Node * current_node = NULL;

  if (_nodal)
    current_node = _current_node;
  else
    current_node = _mesh.getQuadratureNode(_current_elem, _current_side, _qp);

  PenetrationInfo * pinfo = _penetration_locator._penetration_info[current_node->id()];

  Real gap_value = 0.0;

  if (pinfo)
  {
    std::vector<std::vector<Real>> & side_phi = pinfo->_side_phi;
    if (_moose_var.feType().order != CONSTANT)
      gap_value = _moose_var.getValue(pinfo->_side, side_phi);
    else
      gap_value = _moose_var.getValue(pinfo->_elem, side_phi);
  }
  else
  {
    if (_warnings)
    {
      std::stringstream msg;
      msg << "No gap value information found for node ";
      msg << current_node->id();
      msg << " on processor ";
      msg << processor_id();
      mooseWarning(msg.str());
    }
  }
  return gap_value;
}

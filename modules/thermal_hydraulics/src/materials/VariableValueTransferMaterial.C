//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableValueTransferMaterial.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"
#include "Assembly.h"
#include "ADUtils.h"
#include "MooseVariableBase.h"

#include "libmesh/node.h"
#include "libmesh/elem.h"

registerMooseObject("ThermalHydraulicsApp", VariableValueTransferMaterial);

InputParameters
VariableValueTransferMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Creates an AD material property for a variable transferred from the "
                             "boundary of a 2D mesh onto a 1D mesh.");
  params.addRequiredParam<BoundaryName>(
      "secondary_boundary",
      "The boundary coincident with the 1D domain block that we are transferring data to (e.g. the "
      "block this material is executing on).");
  params.addRequiredParam<BoundaryName>("primary_boundary",
                                        "The boundary of the 2D structure  to get the value from.");

  // Have to use std::string to circumvent block restrictable testing
  params.addRequiredParam<std::string>("paired_variable", "The variable to get the value of.");
  params.addRequiredParam<MaterialPropertyName>(
      "property_name",
      "The name of the material property that will be "
      "declared that will represent the transferred variable.");
  return params;
}

VariableValueTransferMaterial::VariableValueTransferMaterial(const InputParameters & parameters)
  : Material(parameters),
    _penetration_locator(getPenetrationLocator(getParam<BoundaryName>("primary_boundary"),
                                               getParam<BoundaryName>("secondary_boundary"),
                                               Order(FIRST))),
    _nearest_node(_penetration_locator._nearest_node),
    _nl_sys(_subproblem.systemBaseNonlinear()),
    _serialized_solution(_nl_sys.currentSolution()),
    _paired_variable(
        _subproblem
            .getVariable(_tid, getParam<std::string>("paired_variable"), Moose::VAR_NONLINEAR)
            .number()),
    _prop(declareADProperty<Real>(getParam<MaterialPropertyName>("property_name"))),
    _phi(_assembly.fePhi<Real>(FEType(FIRST, LAGRANGE)))
{
  _penetration_locator.setCheckWhetherReasonable(false);
}

void
VariableValueTransferMaterial::computeProperties()
{
  std::vector<ADReal> T_w_nodal_values;
  for (const auto i : _current_elem->node_index_range())
  {
    const Node & nd = _current_elem->node_ref(i);

    // Assumes the variable you are coupling to is from the nonlinear system for now.
    const Node * const nearest = _nearest_node.nearestNode(nd.id());
    mooseAssert(nearest, "I do not have the nearest node for you");
    const auto dof_number = nearest->dof_number(_nl_sys.number(), _paired_variable, 0);
    T_w_nodal_values.push_back((*_serialized_solution)(dof_number));
    Moose::derivInsert(T_w_nodal_values.back().derivatives(), dof_number, 1.);
  }

  for (const auto qp : make_range(_qrule->n_points()))
  {
    _prop[qp] = 0;
    for (const auto i : _current_elem->node_index_range())
      _prop[qp] += T_w_nodal_values[i] * _phi[i][qp];
  }
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProjectionAux.h"
#include "SystemBase.h"

registerMooseObjectRenamed("MooseApp", SelfAux, "01/30/2024 24:00", ProjectionAux);
registerMooseObject("MooseApp", ProjectionAux);

InputParameters
ProjectionAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Returns the specified variable as an auxiliary variable with a projection of the source "
      "variable. If they are the same type, this amounts to a simple copy.");
  params.addRequiredCoupledVar("v", "Variable to take the value of.");

  // Technically possible to project from nodal to elemental and back
  params.set<bool>("_allow_nodal_to_elemental_coupling") = true;
  return params;
}

ProjectionAux::ProjectionAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v(isCoupled("v") ? coupledValue("v") : _u),
    _source_variable(getVar("v", 0)),
    _source_sys(_source_variable->sys())
{
}

Real
ProjectionAux::computeValue()
{
  if (!isNodal() || (_source_variable->isNodal() && _source_variable->order() >= _var.order()))
    return _v[_qp];
  // Handle elemental variable projection into a nodal variable
  // AND nodal low order -> nodal higher order
  else
  {
    // Custom projection rule : use neighbor element centroid values weighted by element volumes
    // First, find all the elements that this node is part of
    auto elem_ids = _mesh.nodeToElemMap().find(_current_node->id());
    mooseAssert(elem_ids != _mesh.nodeToElemMap().end(),
                "Should have found an element around node " + std::to_string(_current_node->id()));

    // Get the neighbor element centroid values & element volumes
    std::vector<Real> elem_values(elem_ids->second.size());
    std::vector<Real> elem_volumes(elem_ids->second.size());
    auto index = 0;
    for (auto & id : elem_ids->second)
    {
      const auto & elem = _mesh.elemPtr(id);
      elem_values[index] =
          _source_sys.system().point_value(_source_variable->number(), elem->true_centroid(), elem);
      elem_volumes[index] = elem->volume();
      index++;
    }

    // Average with volume weighting
    Real sum_weighted_values = 0;
    Real sum_volumes = 0;
    for (unsigned int i = 0; i < elem_values.size(); i++)
    {
      sum_weighted_values += elem_values[i] * elem_volumes[i];
      sum_volumes += elem_volumes[i];
    }

    return sum_weighted_values / sum_volumes;
  }
}

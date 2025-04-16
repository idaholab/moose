//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusEssentialBC.h"
#include "AbaqusInputObjects.h"
#include "FEProblem.h"

registerMooseObject("SolidMechanicsApp", AbaqusEssentialBC);

InputParameters
AbaqusEssentialBC::validParams()
{
  InputParameters params = DirichletBCBase::validParams();
  params.addClassDescription(
      "Applies boundary conditions from an Abaqus input read through AbaqusUELMesh");
  params.addRequiredParam<Abaqus::AbaqusID>("abaqus_var_id", "Abaqus variable (DOF) id number.");
  params.addParam<std::string>("abaqus_previous_step", "Step BC values to interpolate from.");
  params.addParam<std::string>("abaqus_step", "If specified take the BC data from the given step");
  // we generate a union of all nodes where any BC applies and suss in the individual case
  // if we really need to set a value for the current variable (shouldApply)
  params.set<std::vector<BoundaryName>>("boundary") = {"abaqus_bc_union_boundary"};
  return params;
}

AbaqusEssentialBC::AbaqusEssentialBC(const InputParameters & parameters)
  : DirichletBCBase(parameters),
    _uel_mesh(
        [this]()
        {
          auto uel_mesh = dynamic_cast<AbaqusUELMesh *>(&_mesh);
          if (!uel_mesh)
            mooseError("Must use an AbaqusUELMesh for UEL support.");
          return uel_mesh;
        }()),
    _abaqus_var_id(getParam<Abaqus::AbaqusID>("abaqus_var_id")),
    _node_value_map_previous(
        isParamValid("abaqus_previous_step")
            ? &_uel_mesh->getBCFor(_abaqus_var_id, getParam<std::string>("abaqus_previous_step"))
            : nullptr),
    _node_value_map(isParamValid("abaqus_step")
                        ? _uel_mesh->getBCFor(_abaqus_var_id, getParam<std::string>("abaqus_step"))
                        : _uel_mesh->getBCFor(_abaqus_var_id))
{
  if (isParamValid("abaqus_previous_step") && !isParamValid("abaqus_step"))
    paramError("abaqus_previous_step", "Must supply `abaqus_step` parameter as well.");
}

bool
AbaqusEssentialBC::shouldApply() const
{
  return _node_value_map.find(_current_node->id()) != _node_value_map.end();
}

Real
AbaqusEssentialBC::computeQpValue()
{
  Real previous = 0.0;
  if (_node_value_map_previous)
  {
    const auto it = _node_value_map_previous->find(_current_node->id());
    if (it != _node_value_map_previous->end())
      previous = it->second;
  }
  const auto current = _node_value_map.at(_current_node->id());

  // we need to compute the current step time (use StepUserObject)
  const auto fraction = _fe_problem.time();

  return previous + (current - previous) * fraction;
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusInputObjects.h"
#include "ExecFlagEnum.h"
#include "Moose.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "AbaqusUELMesh.h"

#include "AbaqusUELStepUserObject.h"
#include "libmesh/libmesh_common.h"
#include <functional>
#include <limits>
#include <algorithm>

registerMooseObject("SolidMechanicsApp", AbaqusUELStepUserObject);

InputParameters
AbaqusUELStepUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Maps the time steps and the load step simulation times. It can be "
      "used in either direction depending on the simulation setup. It "
      "generates steps to be used in StepPeriod to control the enabled/disabled state of objects "
      "with user-provided simulation steps.");

  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  return params;
}

AbaqusUELStepUserObject::AbaqusUELStepUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _times(0),
    _time(_fe_problem.time()),
    _uel_mesh(
        [this]()
        {
          auto uel_mesh = dynamic_cast<AbaqusUELMesh *>(&_fe_problem.mesh());
          if (!uel_mesh)
            mooseError("Must use an AbaqusUELMesh for UEL support.");
          return std::ref(*uel_mesh);
        }()),
    _steps(_uel_mesh.getSteps()),
    _current_step(libMesh::invalid_uint),
    _current_step_fraction(0.0),
    _current_step_bcs({&_uel_mesh.getModel()._bc_var_node_value_map,
                       &_uel_mesh.getModel()._bc_var_node_value_map})
{
  // Fill the time interval look-up table
  _times.resize(_steps.size() + 1);
  for (const auto i : index_range(_times))
    _times[i] = i == 0 ? 0.0 : _steps[i - 1]._duration + _times[i - 1];

  // get a MooseVariable for all variables occuring in *Boundary options
  auto populateVarMap = [this](const Abaqus::Step & step)
  {
    for (const auto & pair : step._bc_var_node_value_map)
    {
      const auto var_id = pair.first;
      if (_var_map.find(var_id) == _var_map.end())
        _var_map[var_id] =
            &UserObject::_subproblem.getVariable(0,
                                                 _uel_mesh.getVarName(var_id),
                                                 Moose::VarKindType::VAR_SOLVER,
                                                 Moose::VarFieldType::VAR_FIELD_STANDARD);
    }
  };

  populateVarMap(_uel_mesh.getModel());
  for (const auto & step : _steps)
    populateVarMap(step);
}

void
AbaqusUELStepUserObject::timestepSetup()
{
  // no steps - nothing to do
  if (_steps.empty())
    return;

  if (_current_execute_flag == EXEC_INITIAL)
    mooseInfoRepeated("EXEC_INITIAL  ", _fe_problem.timeStep());
  if (_current_execute_flag == EXEC_TIMESTEP_BEGIN)
    mooseInfoRepeated("EXEC_TIMESTEP_BEGIN  ", _fe_problem.timeStep());

  // determine current step and check if we changed into a new step
  std::size_t next_step = 0;
  for (; next_step < _times.size() - 1; ++next_step)
  {
    if (_time >= _times[next_step] && _time < _times[next_step + 1])
      break;
  }

  if (next_step == _steps.size())
    mooseError("Simulation ran beyond the final step end time.");

  if (next_step != _current_step)
  {
    if (_current_step != libMesh::invalid_uint && next_step != _current_step + 1)
      mooseError("Cannot step backwards!");

    // shift bc state (beginning and end of new step) forward
    _current_step_bcs = {_current_step_bcs.second, &_steps[next_step]._bc_var_node_value_map};

    // transitioning to a new step
    _current_step = next_step;

    mooseInfoRepeated(_current_step);
    std::cout << "BEGIN\n";
    for (const auto & [k,m] : *_current_step_bcs.first)
    {
      std::cout << "  var " << k << '\n';
      for (const auto & [n,v] : m)
         std::cout << "    n: " << n << " v: " << v << '\n';
    }
    std::cout << "END\n";
    for (const auto & [k,m] : *_current_step_bcs.second)
    {
      std::cout << "  var " << k << '\n';
      for (const auto & [n,v] : m)
         std::cout << "    n: " << n << " v: " << v << '\n';
    }

    const auto & bc_begin = *_current_step_bcs.first;
    const auto & bc_end = *_current_step_bcs.second;

    std::vector<dof_id_type> var_dof_indices;
    std::vector<Real> var_dof_values;

    _current_step_begin_forces.clear();
    _current_step_begin_solution.clear();

    // fetch concentrated forces for all deactivated BCs
    TagID tag = _fe_problem.getVectorTagID("AbaqusUELTag");
    const auto & concentrated_forces = _sys.getVector(tag);
    for (const auto & [var_id, begin_node_value_map] : bc_begin)
    {
      // get the moose variable we're operating on
      const auto & var = *_var_map[var_id];

      // now iterate over all nodes where this variable was active at the beginning of the step
      const auto end_it = bc_end.find(var_id);
      for (const auto & pair : begin_node_value_map)
        // check if the node has no entry in the end_node_value_map
        if (end_it == bc_end.end() || end_it->second.find(pair.first) == end_it->second.end())
        {
          // this is a deactivated BC, so we need to store the concentrated force at the beginning
          // of the timestep
          const auto * node_elem = _uel_mesh.elemPtr(pair.first);
          mooseAssert(node_elem, "Node element not found for UEL element");

          // get DOF index for the node
          var.getDofIndices(node_elem, var_dof_indices);
          mooseAssert(var_dof_indices.size() == 1,
                      "Each variable should have exactly one DOF at each node element");

          // get residual value
          concentrated_forces.get(var_dof_indices, var_dof_values);

          // put into map (we got the concentrated forces from the bulk AbaqusUELMeshUserElement,
          // the nodal reaction force should be the opposite of that - hence the minus sign)
          _current_step_begin_forces[var_id][pair.first] = -var_dof_values[0];

          std::cout << "BEGIN_FORCE " << var_id << ' ' << pair.first << ' ' << -var_dof_values[0] << '\n';
        }
    }

    // fetch current values for all newly activated BCs
    for (const auto & [var_id, end_node_value_map] : bc_end)
    {
      // get the moose variable we're operating on
      const auto & var = *_var_map[var_id];

      // now iterate over all nodes where this variable is activated at the end of the step
      const auto begin_it = bc_begin.find(var_id);
      for (const auto & pair : end_node_value_map)
        // check if the node has no entry in the begin_node_value_map
        if (begin_it == bc_begin.end() ||
            begin_it->second.find(pair.first) == begin_it->second.end())
        {
          // this is a newly activated BC, so we need to store the solution value at the beginning
          // of the timestep
          const auto * node_elem = _uel_mesh.elemPtr(pair.first);
          mooseAssert(node_elem, "Node element not found for UEL element");

          // get DOF index for the node
          var.getDofIndices(node_elem, var_dof_indices);
          mooseAssert(var_dof_indices.size() == 1,
                      "Each variable should have exactly one DOF at each node element");

          // get solution value
          _sys.currentSolution()->get(var_dof_indices, var_dof_values);

          // put into map
          _current_step_begin_solution[var_id][pair.first] = var_dof_values[0];

          std::cout << "BEGIN_SOLUTION " << var_id << ' ' << pair.first << ' ' << var_dof_values[0] << '\n';
        }
    }
  }

  // update step fraction
  if (_current_step != libMesh::invalid_uint)
    _current_step_fraction =
        (_time - _times[_current_step]) / (_times[_current_step + 1] - _times[_current_step]);
}

const std::unordered_map<Abaqus::Index, Real> *
AbaqusUELStepUserObject::getBeginForces(Abaqus::AbaqusID var_id) const
{
  const auto it = _current_step_begin_forces.find(var_id);
  if (it == _current_step_begin_forces.end())
    return nullptr;
  return &it->second;
}

const std::unordered_map<Abaqus::Index, Real> *
AbaqusUELStepUserObject::getBeginSolution(Abaqus::AbaqusID var_id) const
{
  const auto it = _current_step_begin_solution.find(var_id);
  if (it == _current_step_begin_solution.end())
    return nullptr;
  return &it->second;
}

const std::unordered_map<Abaqus::Index, Real> *
AbaqusUELStepUserObject::getBeginValues(Abaqus::AbaqusID var_id) const
{
  const auto it = _current_step_bcs.first->find(var_id);
  if (it == _current_step_bcs.first->end())
    return nullptr;
  return &it->second;
}

const std::unordered_map<Abaqus::Index, Real> *
AbaqusUELStepUserObject::getEndValues(Abaqus::AbaqusID var_id) const
{
  const auto it = _current_step_bcs.second->find(var_id);
  if (it == _current_step_bcs.second->end())
    return nullptr;
  return &it->second;
}

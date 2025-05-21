//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExecFlagEnum.h"
#include "Moose.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "AbaqusUELMesh.h"

#include "AbaqusUELStepUserObject.h"
#include <functional>
#include <limits>
#include <algorithm>

registerMooseObject("SolidMechanicsApp", AbaqusUELStepUserObject);

InputParameters
AbaqusUELStepUserObject::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  params.addClassDescription(
      "Maps the time steps and the load step simulation times. It can be "
      "used in either direction depending on the simulation setup. It "
      "generates steps to be used in StepPeriod to control the enabled/disabled state of objects "
      "with user-provided simulation steps.");
  params.addParam<std::vector<Real>>(
      "step_start_times",
      "The beginning of step times. The number of steps is inferred from the number of times. One "
      "step is defined by its start time; and its end time is taken from the start time of the "
      "next step (if it exists). This list needs to be in ascending value order.");

  params.addParam<Real>("total_time_interval",
                        "The total time interval in which the steps take place. This option needs "
                        "to be used together with the 'number_steps'.");
  params.addParam<unsigned int>(
      "number_steps",
      "Total number of steps in the total time inteval (provided as total_time_interval).");

  params.addParam<std::vector<Real>>(
      "step_durations",
      "The durations of the steps. 'n' of step time intervals define 'n+1' steps "
      "starting at time equals zero.");

  params.addParam<std::vector<NonlinearVariableName>>(
      "bc_variables",
      {},
      "List of variables to store the residual and solution values for to support Abaqus boundary "
      "conditions.");

  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  params.addParam<TagName>("abaqus_bc_tag", "AbaqusBCTag", "Vector tag for the concentrated force vector applied by Abaqus BCs.");
  return params;
}

AbaqusUELStepUserObject::AbaqusUELStepUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _times(0),
    _time(_fe_problem.time()),
    _current_step(libMesh::invalid_uint),
    _uel_mesh(
        [this]()
        {
          auto uel_mesh = dynamic_cast<AbaqusUELMesh *>(&_fe_problem.mesh());
          if (!uel_mesh)
            mooseError("Must use an AbaqusUELMesh for UEL support.");
          return std::ref(*uel_mesh);
        }()),
    _steps(_uel_mesh.getSteps()),
    _concentrated_forces(_sys.getVector(getParam<TagName>("abaqus_bc_tag")))
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
                                                 Moose::VarKindType::VAR_AUXILIARY,
                                                 Moose::VarFieldType::VAR_FIELD_STANDARD);
    }
  };

  populateVarMap(_uel_mesh.getModel());
  for (const auto & step : _steps)
    populateVarMap(step);
}

void
AbaqusUELStepUserObject::initialize()
{
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

  if (next_step != _current_step)
  {
    const static std::unordered_map<Abaqus::AbaqusID, std::unordered_map<Abaqus::Index, Real>> empty;
    const auto & data = _steps[next_step]._bc_var_node_value_map;

    // transitioning to a new step
    _current_step = next_step;

    // prepare IC data
    _bc_values.clear();
    _bc_forces.clear();

    // fetch end values from the UEL mesh and corresponding start values from the solution vector
    for (const auto & [var_id, node_value_map] : data)
    {
    }
  }
}

void
AbaqusUELStepUserObject::execute()
{
}

void
AbaqusUELStepUserObject::finalize()
{
}

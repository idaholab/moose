//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusInputObjects.h"
#include "AbaqusUELMesh.h"
#include "AbaqusUELStepUserObject.h"
#include "AbaqusPredefAux.h"
#include "Function.h"

registerMooseObject("SolidMechanicsApp", AbaqusPredefAux);

InputParameters
AbaqusPredefAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Make an Abaqus Field Initial Condition available as an AuxVariable");

  params.addRequiredRangeCheckedParam<Abaqus::AbaqusID>(
      "field",
      "field >= 1",
      "Abaqus field number (starting with 1) from the `variable=` parameter of the `*Initial "
      "Condition` section in the Abaqus input.");
  params.addParam<UserObjectName>(
      "step_user_object",
      "Step user object providing time-varying `*Field` values for the current step. Only "
      "needed if the Abaqus input assigns this field with a step-level `*Field` option; "
      "otherwise the field stays fixed at its `*Initial Conditions` value.");

  // default this object to run on initial, and at the beginning of each timestep so that
  // *Field values driven by an amplitude table or a step ramp stay current
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};

  return params;
}

AbaqusPredefAux::AbaqusPredefAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _uel_mesh(dynamic_cast<AbaqusUELMesh *>(&_mesh)),
    _var_id(getParam<Abaqus::AbaqusID>("field")),
    _step_uo(isParamValid("step_user_object")
                 ? &getUserObject<AbaqusUELStepUserObject>("step_user_object")
                 : nullptr)
{
  if (!_uel_mesh)
    mooseError("Must use an AbaqusUELMesh for UEL support.");

  if (!isNodal())
    paramError("variable", "Must be a nodal variable");
}

void
AbaqusPredefAux::initialSetup()
{
  update();
}

void
AbaqusPredefAux::timestepSetup()
{
  update();
}

void
AbaqusPredefAux::update()
{
  _ic_data.clear();

  for (const auto & ic : _uel_mesh->getFieldICs())
    if (ic._var == _var_id)
    {
      for (const auto & [nodeset_name, value] : ic._value)
        for (const auto node_index : ic._nsets.at(nodeset_name))
          _ic_data[node_index] = value;
      return;
    }
  paramWarning("field", "No field `*Initial condition` block found for variable=", _var_id);
}

Real
AbaqusPredefAux::computeValue()
{
  // *Initial Conditions baseline (holds for the duration of the analysis unless overridden below)
  const auto ic_it = _ic_data.find(_current_node->id());
  Real value = ic_it != _ic_data.end() ? ic_it->second : 0.0;

  if (!_step_uo)
    return value;

  // overlay the current step's *Field assignment for this node, if any
  const auto * end_fields = _step_uo->getEndFields(_var_id);
  if (!end_fields)
    return value;
  const auto end_it = end_fields->find(_current_node->id());
  if (end_it == end_fields->end())
    return value;
  const auto & assignment = end_it->second;

  if (!assignment._amplitude.empty())
    // amplitude(t) scaled by the value given on the *Field line
    return getFunctionByName(Abaqus::amplitudeFunctionName(assignment._amplitude))
               .value(_t, Point()) *
           assignment._value;

  // no amplitude: linearly ramp from the value at the beginning of the step (the value assigned
  // in a prior step, or the *Initial Conditions/zero baseline if this is the first assignment)
  // to the value assigned here, over the step - same convention as *Boundary.
  Real begin_value = value;
  const auto * begin_fields = _step_uo->getBeginFields(_var_id);
  if (begin_fields)
  {
    const auto begin_it = begin_fields->find(_current_node->id());
    if (begin_it != begin_fields->end())
      begin_value = begin_it->second._value;
  }
  const Real d = _step_uo->getStepFraction();
  return begin_value + d * (assignment._value - begin_value);
}

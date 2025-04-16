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
#include "AbaqusPredefAux.h"

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

  // default this object to run on initial
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL};

  return params;
}

AbaqusPredefAux::AbaqusPredefAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _uel_mesh(dynamic_cast<AbaqusUELMesh *>(&_mesh)),
    _var_id(getParam<Abaqus::AbaqusID>("field"))
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
  const auto it = _ic_data.find(_current_node->id());
  if (it != _ic_data.end())
    return it->second;
  return 0.0;
}

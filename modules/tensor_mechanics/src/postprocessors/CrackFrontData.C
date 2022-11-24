//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackFrontData.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

#include "libmesh/boundary_info.h"

registerMooseObject("TensorMechanicsApp", CrackFrontData);

InputParameters
CrackFrontData::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Determines which nodes are along the crack front");
  params.addRequiredParam<VariableName>(
      "variable", "The name of a variable whose value at the crack front is to be reported");
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  params.addParam<unsigned int>(
      "crack_front_point_index",
      "The index of the point on the crack front where data is to be reported");
  params.addParam<Real>("scale_factor", 1, "A scale factor to be applied to the reported quantity");
  return params;
}

CrackFrontData::CrackFrontData(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _crack_front_point_index(isParamValid("crack_front_point_index")
                                 ? getParam<unsigned int>("crack_front_point_index")
                                 : 0),
    _crack_front_node(NULL),
    _mesh(_subproblem.mesh()),
    _var_name(parameters.get<VariableName>("variable")),
    _scale_factor(getParam<Real>("scale_factor")),
    _field_var(_subproblem.getStandardVariable(_tid, _var_name)),
    _value(0)
{
  if (!_field_var.isNodal())
    mooseError("CrackFrontData can be output only for nodal variables, variable '",
               _var_name,
               "' is not nodal");
}

void
CrackFrontData::initialize()
{
  if (!(_crack_front_point_index < _crack_front_definition->getNumCrackFrontPoints()))
    mooseError("crack_front_point_index out of range in CrackFrontData");
  if (!_crack_front_definition->hasCrackFrontNodes())
    mooseError("CrackFrontData not currently supported if crack front is defined with points "
               "rather than nodes");

  _crack_front_node = _crack_front_definition->getCrackFrontNodePtr(_crack_front_point_index);
}
void
CrackFrontData::execute()
{
  _value = 0;

  if (_crack_front_node->processor_id() == processor_id())
    _value = _field_var.getNodalValue(*_crack_front_node);
}

Real
CrackFrontData::getValue()
{

  return _scale_factor * _value;
}

void
CrackFrontData::finalize()
{
  gatherSum(_value);
}

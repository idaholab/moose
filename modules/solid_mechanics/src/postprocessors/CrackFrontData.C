/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CrackFrontData.h"
#include "MooseMesh.h"
#include "SubProblem.h"

// libMesh
#include "libmesh/boundary_info.h"

template <>
InputParameters
validParams<CrackFrontData>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
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
    _scale_factor(getParam<Real>("scale_factor"))
{
  if (!_subproblem.getVariable(_tid, _var_name).isNodal())
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

Real
CrackFrontData::getValue()
{
  Real value = 0;

  if (_crack_front_node->processor_id() == processor_id())
    value = _subproblem.getVariable(_tid, _var_name).getNodalValue(*_crack_front_node);

  gatherSum(value);

  return _scale_factor * value;
}

#include "TriSubChannelPointValue.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "libmesh/system.h"

registerMooseObject("SubChannelApp", TriSubChannelPointValue);

InputParameters
TriSubChannelPointValue::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable", "Variable you want the value of");
  params.addRequiredParam<Real>("height", "Axial location of point [m]");
  params.addRequiredParam<int>("index", "Index of subchannel");
  params.addClassDescription(
      "Prints out a user selected value of specified sub-channel at a user selected axial height");
  return params;
}

TriSubChannelPointValue::TriSubChannelPointValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(dynamic_cast<TriSubChannelMesh &>(_fe_problem.mesh())),
    _variable(getParam<VariableName>("variable")),
    _height(getParam<Real>("height")),
    _i_ch(getParam<int>("index")),
    _var_number(_subproblem
                    .getVariable(_tid,
                                 parameters.get<VariableName>("variable"),
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD)
                    .number()),
    _system(_subproblem.getSystem(getParam<VariableName>("variable"))),
    _value(0)
{
  _point =
      Point(_mesh._subchannel_position[_i_ch][0], _mesh._subchannel_position[_i_ch][1], _height);
}

void
TriSubChannelPointValue::execute()
{
  _value = _system.point_value(_var_number, _point, false);

  if (MooseUtils::absoluteFuzzyEqual(_value, 0.0))
  {
    auto pl = _subproblem.mesh().getPointLocator();
    pl->enable_out_of_mesh_mode();
    auto * elem = (*pl)(_point);
    auto elem_id = elem ? elem->id() : DofObject::invalid_id;
    gatherMin(elem_id);
    if (elem_id == DofObject::invalid_id)
      mooseError(name(), ": No element located at ", _point, ".");
  }
}

Real
TriSubChannelPointValue::getValue()
{
  return _value;
}

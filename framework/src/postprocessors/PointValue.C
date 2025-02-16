//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointValue.h"

// MOOSE includes
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

#include "libmesh/system.h"

registerMooseObject("MooseApp", PointValue);

InputParameters
PointValue::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Compute the value of a variable at a specified location.");
  params.addRequiredParam<VariableName>(
      "variable", "The name of the variable that this postprocessor operates on.");
  params.addRequiredParam<Point>("point",
                                 "The physical point where the solution will be evaluated.");
  params.addClassDescription("Compute the value of a variable at a specified location");
  return params;
}

PointValue::PointValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _var_number(_subproblem
                    .getVariable(_tid,
                                 parameters.get<VariableName>("variable"),
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD)
                    .number()),
    _system(_subproblem.getSystem(getParam<VariableName>("variable"))),
    _point(getParam<Point>("point")),
    _value(0)
{
}

void
PointValue::execute()
{
  _value = _system.point_value(_var_number, _point, false);

  /**
   * If we get exactly zero, we don't know if the locator couldn't find an element, or
   * if the solution is truly zero, more checking is needed.
   */
  if (MooseUtils::absoluteFuzzyEqual(_value, 0.0))
  {
    auto pl = _subproblem.mesh().getPointLocator();
    pl->enable_out_of_mesh_mode();

    auto * elem = (*pl)(_point);
    auto elem_id = elem ? elem->id() : libMesh::DofObject::invalid_id;
    gatherMin(elem_id);

    if (elem_id == libMesh::DofObject::invalid_id)
      mooseError("No element located at ", _point, " in PointValue Postprocessor named: ", name());
  }
}

Real
PointValue::getValue() const
{
  return _value;
}

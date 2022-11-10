/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "QuadSubChannelPointValue.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "libmesh/system.h"

registerMooseObject("SubChannelApp", QuadSubChannelPointValue);

InputParameters
QuadSubChannelPointValue::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable", "Variable you want the value of");
  params.addRequiredParam<Real>("height", "Axial location of normal slice [m]");
  params.addRequiredParam<int>("ix",
                               "Location of sub_channel in the x direction, 0 being first [-]");
  params.addRequiredParam<int>("iy",
                               "Location of sub_channel in the y direction, 0 being first [-]");
  params.addClassDescription(
      "Prints out a user selected value of specified sub-channel at a user selected axial height");
  return params;
}

QuadSubChannelPointValue::QuadSubChannelPointValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(dynamic_cast<QuadSubChannelMesh &>(_fe_problem.mesh())),
    _variable(getParam<VariableName>("variable")),
    _height(getParam<Real>("height")),
    _ix(getParam<int>("ix")),
    _iy(getParam<int>("iy")),
    _var_number(_subproblem
                    .getVariable(_tid,
                                 parameters.get<VariableName>("variable"),
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD)
                    .number()),
    _system(_subproblem.getSystem(getParam<VariableName>("variable"))),
    _value(0)
{
  auto nx = _mesh.getNx();
  auto ny = _mesh.getNy();
  auto pitch = _mesh.getPitch();
  Real x = (_ix - (nx - 1) / 2) * pitch;
  Real y = (_iy - (ny - 1) / 2) * pitch;
  _point = Point(x, y, _height);
}

void
QuadSubChannelPointValue::execute()
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
QuadSubChannelPointValue::getValue()
{
  return _value;
}

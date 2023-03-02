//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceQpValueUserObject.h"

registerMooseObject("MooseApp", InterfaceQpValueUserObject);

InputParameters
InterfaceQpValueUserObject::validParams()
{
  InputParameters params = InterfaceQpUserObjectBase::validParams();
  params.addRequiredCoupledVar("var", "The variable name");
  params.addCoupledVar("var_neighbor", "The neighbor variable name");
  params.addClassDescription(
      "Computes the variable value, rate or increment across an "
      "interface. The value, rate or increment is computed according to the provided "
      "interface_value_type parameter");
  return params;
}

InterfaceQpValueUserObject::InterfaceQpValueUserObject(const InputParameters & parameters)
  : InterfaceQpUserObjectBase(parameters),
    _u(_value_type > 0 ? coupledDot("var") : coupledValue("var")),
    _u_neighbor(
        parameters.isParamSetByUser("var_neighbor")
            ? (_value_type > 0 ? coupledNeighborValueDot("var_neighbor")
                               : coupledNeighborValue("var_neighbor"))
            : (_value_type > 0 ? coupledNeighborValueDot("var") : coupledNeighborValue("var")))

{
}

Real
InterfaceQpValueUserObject::computeRealValue(const unsigned int qp)
{
  /* civet complains about fall through, let's fix this using some extra code */
  switch (_value_type)
  {
    case 0: /*value*/
      return computeInterfaceValueType(_u[qp], _u_neighbor[qp]);
    case 1: /*rate*/
      return computeInterfaceValueType(_u[qp], _u_neighbor[qp]);
    case 2: /*increment*/
      return computeInterfaceValueType(_u[qp] * _dt, _u_neighbor[qp] * _dt);
    default:
      mooseError("InterfaceQpValueUserObject::computeRealValue the supplied "
                 "value type has not been implemented");
  }
  mooseError("InterfaceQpValueUserObject::computeRealValue if we are here something is wrong");
}

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

defineLegacyParams(InterfaceQpValueUserObject);

InputParameters
InterfaceQpValueUserObject::validParams()
{
  InputParameters params = InterfaceQpUserObjectBase::validParams();
  params.addRequiredCoupledVar("var", "The variable name");
  params.addCoupledVar("var_neighbor", "The neighbor variable name");
  params.addClassDescription("Computes the variable value, rate or increment across an "
                             "interface. The value or rate is computed according to the provided "
                             "interface_value_type parameter");
  return params;
}

InterfaceQpValueUserObject::InterfaceQpValueUserObject(const InputParameters & parameters)
  : InterfaceQpUserObjectBase(parameters),
    _u(_compute_rate || _compute_increment ? coupledDot("var") : coupledValue("var")),
    _u_neighbor(parameters.isParamSetByUser("var_neighbor")
                    ? (_compute_rate || _compute_increment ? coupledNeighborValueDot("var_neighbor")
                                                           : coupledNeighborValue("var_neighbor"))
                    : (_compute_rate || _compute_increment ? coupledNeighborValueDot("var")
                                                           : coupledNeighborValue("var")))

{
}

Real
InterfaceQpValueUserObject::computeRealValue(const unsigned int qp)
{
  Real val = computeInterfaceValueType(_u[qp], _u_neighbor[qp]);
  if (_compute_increment)
    val *= _dt;

  return val;
}

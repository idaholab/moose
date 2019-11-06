//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatTestNeumannBC.h"

registerMooseObject("MooseTestApp", MatTestNeumannBC);

InputParameters
MatTestNeumannBC::validParams()
{
  InputParameters p = NeumannBC::validParams();
  p.addRequiredParam<std::string>("mat_prop",
                                  "The material property that gives the value of the BC");
  p.addParam<bool>("has_check", false, "Test hasActiveBoundaryObjects method.");
  return p;
}

MatTestNeumannBC::MatTestNeumannBC(const InputParameters & parameters)
  : NeumannBC(parameters), _prop_name(getParam<std::string>("mat_prop"))
{
  if (getParam<bool>("has_check") && !hasBoundaryMaterialProperty<Real>(_prop_name))
    mooseError(
        "The material property ", _prop_name, " is not defined on all boundaries of this object");

  _value = &getMaterialPropertyByName<Real>(_prop_name);
}

Real
MatTestNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * (*_value)[_qp];
}

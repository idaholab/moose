//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVArrayCoupledForce.h"

#include "MooseVariable.h"

registerMooseObject("MooseTestApp", FVArrayCoupledForce);

InputParameters
FVArrayCoupledForce::validParams()
{
  InputParameters params = FVArrayElementalKernel::validParams();
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  params.addRequiredParam<RealEigenVector>(
      "coef", "Coefficent ($\\sigma$) multiplier for the coupled force term.");
  params.addClassDescription(
      "Implements a source term proportional to the value of a coupled "
      "standard variable. Weak form: $(\\vec{u}^\\ast, -\\vec{\\sigma} v)$.");
  return params;
}

FVArrayCoupledForce::FVArrayCoupledForce(const InputParameters & parameters)
  : FVArrayElementalKernel(parameters), _v(adCoupledValue("v"))
{
  auto & coef = getParam<RealEigenVector>("coef");
  _coef.resize(coef.size());
  for (int i = 0; i < coef.size(); i++)
    _coef(i) = coef(i);

  if (_var.number() == coupled("v"))
    mooseError("Coupled variable 'v' needs to be different from 'variable' with CoupledForce, "
               "consider using Reaction or somethig similar");
  if (getFieldVar("v", 0)->count() > 1)
    mooseError("We are testing the coupling of a standard variable to an array variable");
}

ADRealEigenVector
FVArrayCoupledForce::computeQpResidual()
{
  return -_coef * _v[_qp];
}

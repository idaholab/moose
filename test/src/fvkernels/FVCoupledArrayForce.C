//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVCoupledArrayForce.h"

#include "MooseVariable.h"

registerMooseObject("MooseTestApp", FVCoupledArrayForce);

InputParameters
FVCoupledArrayForce::validParams()
{
  InputParameters params = FVElementalKernel::validParams();

  params.addRequiredCoupledVar("v", "The coupled array variable which provides the force");
  params.addRequiredParam<RealEigenVector>(
      "coef", "Coefficent ($\\sigma$) multiplier for the coupled force term.");
  params.addClassDescription("Implements a source term proportional to the value of a coupled "
                             "array variable. Weak form: $(u^\\ast, -\\vec{\\sigma} \\vec{v})$.");
  return params;
}

FVCoupledArrayForce::FVCoupledArrayForce(const InputParameters & parameters)
  : FVElementalKernel(parameters), _v(adCoupledArrayValue("v"))
{
  auto & coef = getParam<RealEigenVector>("coef");
  _coef.resize(coef.size());
  for (int i = 0; i < coef.size(); i++)
    _coef(i) = coef(i);

  if (_var.number() == coupled("v"))
    mooseError("Coupled variable 'v' needs to be different from 'variable' with CoupledForce, "
               "consider using Reaction or somethig similar");
  if (getArrayVar("v", 0)->count() != coef.size())
    mooseError(
        "'coef' size in FVCoupledArrayForce is inconsistent with the number of components of "
        "the coupled array variable");
}

ADReal
FVCoupledArrayForce::computeQpResidual()
{
  return -(_coef.transpose() * _v[_qp]).sum();
}

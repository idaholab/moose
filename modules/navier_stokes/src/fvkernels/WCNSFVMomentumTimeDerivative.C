//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVMomentumTimeDerivative.h"
#include "SystemBase.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVMomentumTimeDerivative);

InputParameters
WCNSFVMomentumTimeDerivative::validParams()
{
  InputParameters params = INSFVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes momentum equation.");
  params.addRequiredParam<MaterialPropertyName>(NS::density, "The density material property");
  params.addRequiredParam<MaterialPropertyName>(
      NS::time_deriv(NS::density), "The time derivative of the density material property");
  return params;
}

WCNSFVMomentumTimeDerivative::WCNSFVMomentumTimeDerivative(const InputParameters & params)
  : INSFVTimeKernel(params),
    _rho(getFunctor<ADReal>(NS::density)),
    _rho_dot(getFunctor<ADReal>(NS::time_deriv(NS::density)))
{
}

void
WCNSFVMomentumTimeDerivative::gatherRCData(const Elem & elem)
{
  // _rho and _rho_dot could ultimately be functions of the nonlinear variables making our residual
  // nonlinear so we cannot do the simple treatment that is done in
  // INSFVMomentumTimeDerivative::gatherRCData

  const auto elem_arg = makeElemArg(&elem);
  const auto rho_dot = _rho_dot(elem_arg);
  const auto var_dot = _var.dot(elem_arg);
  const auto rho = _rho(elem_arg);
  const auto var = _var(elem_arg);

  const auto dof_number = elem.dof_number(_sys.number(), _var.number(), 0);
  mooseAssert(var.derivatives()[dof_number] == 1.,
              "This is an implicit assumption in our coefficient calculation.");

  const auto strong_resid = rho_dot * var + rho * var_dot;

  // For the first term in the above strong residual we know that var.derivatives()[dof_number] = 1
  // so there is no need to explicitly index here
  ADReal a = rho_dot;
  // but there is a need here
  a += rho * var_dot.derivatives()[dof_number];

  const auto volume = _assembly.elementVolume(&elem);
  _rc_uo.addToA(&elem, _index, a * volume);
  processResidual(strong_resid * volume, dof_number);
}

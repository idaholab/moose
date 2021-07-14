//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSEntropyError.h"
#include "NS.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

registerMooseObject("NavierStokesApp", NSEntropyError);

InputParameters
NSEntropyError::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addClassDescription("Computes entropy error.");
  params.addRequiredParam<Real>("rho_infty", "Freestream density");
  params.addRequiredParam<Real>("p_infty", "Freestream pressure");
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar(NS::pressure, "pressure");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");
  return params;
}

NSEntropyError::NSEntropyError(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _rho_infty(getParam<Real>("rho_infty")),
    _p_infty(getParam<Real>("p_infty")),
    _rho(coupledValue("rho")),
    _pressure(coupledValue(NS::pressure)),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

Real
NSEntropyError::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
NSEntropyError::computeQpIntegral()
{
  Real integrand = (_pressure[_qp] / _p_infty) * std::pow(_rho_infty / _rho[_qp], _fp.gamma()) - 1.;
  return integrand * integrand;
}

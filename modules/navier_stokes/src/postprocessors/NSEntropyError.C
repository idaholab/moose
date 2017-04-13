/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NSEntropyError.h"
#include "NS.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

template <>
InputParameters
validParams<NSEntropyError>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addClassDescription("Computes entropy error.");
  params.addRequiredParam<Real>("rho_infty", "Freestream density");
  params.addRequiredParam<Real>("p_infty", "Freestream pressure");
  params.addRequiredCoupledVar(NS::density, "density");
  params.addRequiredCoupledVar(NS::pressure, "pressure");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");
  return params;
}

NSEntropyError::NSEntropyError(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _rho_infty(getParam<Real>("rho_infty")),
    _p_infty(getParam<Real>("p_infty")),
    _rho(coupledValue(NS::density)),
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

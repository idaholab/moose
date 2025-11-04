//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVMomentumFriction.h"
#include "NS.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", LinearFVMomentumFriction);

InputParameters
LinearFVMomentumFriction::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription("Computes a Darcy friction force term on fluid in the "
                             "Navier Stokes i-th momentum equation.");
  params.addParam<MooseFunctorName>("Darcy_name", "Name of the Darcy coefficients property.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "The dynamic viscosity");

  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");

  return params;
}

LinearFVMomentumFriction::LinearFVMomentumFriction(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _index(getParam<MooseEnum>("momentum_component")),
    _D(isParamValid("Darcy_name") ? &getFunctor<RealVectorValue>("Darcy_name") : nullptr),
    _mu(isParamValid(NS::mu) ? &getFunctor<Real>(NS::mu) : nullptr)
{
}

Real
LinearFVMomentumFriction::computeMatrixContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();
  return computeFrictionWCoefficient(elem_arg, state) * _current_elem_volume;
}

Real
LinearFVMomentumFriction::computeRightHandSideContribution()
{
  return 0;
}

Real
LinearFVMomentumFriction::computeFrictionWCoefficient(const Moose::ElemArg & elem_arg,
                                                      const Moose::StateArg & state)
{
  return (*_mu)(elem_arg, state) * (*_D)(elem_arg, state)(_index);
}

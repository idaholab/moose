//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumTimeDerivative.h"
#include "SystemBase.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMomentumTimeDerivative);

InputParameters
INSFVMomentumTimeDerivative::validParams()
{
  InputParameters params = INSFVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes momentum equation.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density functor");
  return params;
}

INSFVMomentumTimeDerivative::INSFVMomentumTimeDerivative(const InputParameters & params)
  : INSFVTimeKernel(params), _rho(getFunctor<ADReal>(NS::density))
{
}

void
INSFVMomentumTimeDerivative::gatherRCData(const Elem & elem)
{
  const auto e = makeElemArg(&elem);
  const auto state = determineState();
  const auto residual = _rho(e, state) * _var.dot(e, state) * _assembly.elementVolume(&elem);
  const auto dof_number = elem.dof_number(_sys.number(), _var.number(), 0);
  const Real a = residual.derivatives()[dof_number];

  if (_contribute_to_rc_coeffs)
    _rc_uo.addToA(&elem, _index, a);

  addResidualAndJacobian(residual, dof_number);
}

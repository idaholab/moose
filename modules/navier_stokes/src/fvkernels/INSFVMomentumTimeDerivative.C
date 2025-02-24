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
#include "ImplicitEuler.h"

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
  // Check time integrator
  if (_use_fixed_dt_rc_contrib)
    if (!dynamic_cast<const ImplicitEuler *>(
            _fe_problem.getNonlinearSystemBase(_sys.number()).queryTimeIntegrator(_var.number())))
      paramError("fixed_dt_contribution_to_RC", "Only ImplicitEuler time integrator is supported");
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
  {
    if (_use_fixed_dt_rc_contrib && _fe_problem.time() > _fixed_dt_rc_contrib_start)
      // Change the time derivative contribution to only depend on the fixed dt
      _rc_uo.addToA(&elem, _index, a * _dt / _fixed_dt_rc_contrib);
    else
      _rc_uo.addToA(&elem, _index, a);
  }

  addResidualAndJacobian(residual, dof_number);
}

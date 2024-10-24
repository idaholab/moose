//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVEnergyAdvection.h"
#include "INSFVEnergyVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVEnergyAdvection);

InputParameters
INSFVEnergyAdvection::validParams()
{
  auto params = INSFVAdvectionKernel::validParams();
  params.addClassDescription("Advects energy, e.g. rho*cp*T. A user may still override what "
                             "quantity is advected, but the default is rho*cp*T");
  params.addParam<MooseFunctorName>(
      "advected_quantity", NS::enthalpy_density, "The heat quantity to advect.");
  return params;
}

INSFVEnergyAdvection::INSFVEnergyAdvection(const InputParameters & params)
  : INSFVAdvectionKernel(params), _adv_quant(getFunctor<ADReal>("advected_quantity"))
{
  if (!dynamic_cast<INSFVEnergyVariable *>(&_var))
    mooseError("PINSFVEnergyAdvection may only be used with a fluid temperature variable, "
               "of variable type INSFVEnergyVariable.");
}

ADReal
INSFVEnergyAdvection::computeQpResidual()
{
  const auto v = velocity();
  const auto & limiter_time = _subproblem.isTransient()
                                  ? Moose::StateArg(1, Moose::SolutionIterationType::Time)
                                  : Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);
  const auto adv_quant_face = _adv_quant(makeFace(*_face_info,
                                                  limiterType(_advected_interp_method),
                                                  MetaPhysicL::raw_value(v) * _normal > 0,
                                                  false,
                                                  &limiter_time),
                                         determineState());
  return _normal * v * adv_quant_face;
}

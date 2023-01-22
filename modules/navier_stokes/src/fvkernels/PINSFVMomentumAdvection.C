//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumAdvection.h"
#include "FVUtils.h"
#include "MathFVUtils.h"
#include "NS.h"
#include "INSFVRhieChowInterpolator.h"
#include "BernoulliPressureVariable.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumAdvection);

InputParameters
PINSFVMomentumAdvection::validParams()
{
  auto params = INSFVMomentumAdvection::validParams();
  params.addClassDescription("Object for advecting superficial momentum, e.g. rho*u_d, "
                             "in the porous media momentum equation");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity");
  return params;
}

PINSFVMomentumAdvection::PINSFVMomentumAdvection(const InputParameters & params)
  : INSFVMomentumAdvection(params), _eps(getFunctor<ADReal>(NS::porosity))
{
  const auto & pressure_var = _rc_vel_provider.pressure(_tid);
  if (dynamic_cast<const BernoulliPressureVariable *>(&pressure_var) && _tid == 0)
    adjustRMGhostLayers(std::max((unsigned short)(3), _pars.get<unsigned short>("ghost_layers")));
}

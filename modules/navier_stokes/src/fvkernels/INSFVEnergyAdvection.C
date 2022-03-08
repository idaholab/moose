//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVEnergyAdvection.h"

registerMooseObject("NavierStokesApp", INSFVEnergyAdvection);

InputParameters
INSFVEnergyAdvection::validParams()
{
  auto params = INSFVAdvectionKernel::validParams();
  params.addClassDescription("Advects energy, e.g. rho*cp*T. A user may still override what "
                             "quantity is advected, but the default is rho*cp*T");
  params.addParam<MooseFunctorName>(
      "advected_quantity", "rho_cp_temp", "The heat quantity to advect.");
  return params;
}

INSFVEnergyAdvection::INSFVEnergyAdvection(const InputParameters & params)
  : INSFVAdvectionKernel(params), _adv_quant(getFunctor<ADReal>("advected_quantity"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
INSFVEnergyAdvection::computeQpResidual()
{
  const auto v = _rc_vel_provider.getVelocity(_velocity_interp_method, *_face_info, _tid);
  const auto adv_quant_interface =
      Moose::FV::interpolate(_adv_quant,
                             Moose::FV::makeFace(*_face_info,
                                                 limiterType(_advected_interp_method),
                                                 MetaPhysicL::raw_value(v) * _normal > 0,
                                                 faceArgSubdomains()));
  return _normal * v * adv_quant_interface;
}

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
  auto params = INSFVMomentumAdvection::validParams();
  params.addClassDescription("Advects energy, e.g. rho*cp*T. A user may still override what "
                             "quantity is advected, but the default is rho*cp*T");
  params.addParam<MooseFunctorName>(
      "advected_quantity", "rho_cp_temp", "The heat quantity to advect.");
  params.suppressParameter<MooseEnum>("momentum_component");
  return params;
}

INSFVEnergyAdvection::INSFVEnergyAdvection(const InputParameters & params)
  : INSFVMomentumAdvection(params), _adv_quant(getFunctor<ADReal>("advected_quantity"))
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
  ADReal adv_quant_interface;

  const auto elem_face = elemFromFace();
  const auto neighbor_face = neighborFromFace();

  const auto v = _rc_uo.getVelocity(_velocity_interp_method, *_face_info, _tid);
  Moose::FV::interpolate(_advected_interp_method,
                         adv_quant_interface,
                         _adv_quant(elem_face),
                         _adv_quant(neighbor_face),
                         v,
                         *_face_info,
                         true);
  return _normal * v * adv_quant_interface;
}

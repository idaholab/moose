//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumAdvection.h"
#include "INSFVPressureVariable.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "FVUtils.h"
#include "MathFVUtils.h"
#include "NS.h"
#include "INSFVRhieChowInterpolator.h"

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
  if (!dynamic_cast<const PINSFVSuperficialVelocityVariable *>(_u_var))
    mooseError("PINSFVMomentumAdvection may only be used with a superficial advective velocity, "
               "of variable type PINSFVSuperficialVelocityVariable.");
}

ADReal
PINSFVMomentumAdvection::computeQpResidual()
{
  const auto elem_face = elemFromFace();
  const auto neighbor_face = neighborFromFace();

  // Superficial velocity interpolation
  const auto v = _rc_uo.getVelocity(_velocity_interp_method, *_face_info, _tid);

  const auto interp_coeffs = Moose::FV::interpCoeffs(_advected_interp_method, *_face_info, true, v);

  _ae = _normal * v * _rho(elem_face) * interp_coeffs.first / _eps(elem_face);
  // Minus sign because we apply a minus sign to the residual in computeResidual
  _an = -_normal * v * _rho(neighbor_face) * interp_coeffs.second / _eps(neighbor_face);

  return _ae * _var(elem_face) - _an * _var(neighbor_face);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMassAdvection.h"
#include "NS.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", PINSFVMassAdvection);

InputParameters
PINSFVMassAdvection::validParams()
{
  auto params = INSFVMassAdvection::validParams();
  params.addClassDescription("Object for advecting mass in porous media mass equation");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "The porosity");
  return params;
}

PINSFVMassAdvection::PINSFVMassAdvection(const InputParameters & params)
  : INSFVMassAdvection(params), _eps(getFunctor<ADReal>(NS::porosity))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
PINSFVMassAdvection::computeQpResidual()
{
  const auto [v_face, rho_face] = [this]() -> std::pair<ADRealVectorValue, ADReal>
  {
    if (onBoundary(*_face_info))
      return {_rc_vel_provider.getVelocity(_velocity_interp_method, *_face_info, _tid),
              _rho(singleSidedFaceArg())};
    else if (std::get<0>(NS::isPorosityJumpFace(_eps, *_face_info)))
    {
      const Moose::SingleSidedFaceArg ssf_elem{
          _face_info, Moose::FV::LimiterType::CentralDifference, true, false, &_face_info->elem()};
      const Moose::SingleSidedFaceArg ssf_neighbor{_face_info,
                                                   Moose::FV::LimiterType::CentralDifference,
                                                   true,
                                                   false,
                                                   _face_info->neighborPtr()};

      const auto v_face = _rc_vel_provider.getUpwindSingleSidedFaceVelocity(*_face_info, _tid);
      const bool fi_elem_is_upwind = v_face * _normal > 0;
      const auto & upwind_ssf = fi_elem_is_upwind ? ssf_elem : ssf_neighbor;
      const auto rho_face = _rho(upwind_ssf);
      return {v_face, rho_face};
    }
    else
    {
      const auto v_face = _rc_vel_provider.getVelocity(_velocity_interp_method, *_face_info, _tid);
      return {v_face,
              _rho(Moose::FV::makeFace(*_face_info,
                                       limiterType(_advected_interp_method),
                                       MetaPhysicL::raw_value(v_face) * _normal > 0,
                                       *this))};
    }
  }();

  return _normal * v_face * rho_face;
}

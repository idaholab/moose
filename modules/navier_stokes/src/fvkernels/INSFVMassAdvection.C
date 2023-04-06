//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMassAdvection.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMassAdvection);

InputParameters
INSFVMassAdvection::validParams()
{
  auto params = INSFVAdvectionKernel::validParams();
  params.addClassDescription("Object for advecting mass, e.g. rho");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density functor");
  return params;
}

INSFVMassAdvection::INSFVMassAdvection(const InputParameters & params)
  : INSFVAdvectionKernel(params), _rho(getFunctor<ADReal>(NS::density))
{
}

ADReal
INSFVMassAdvection::computeQpResidual()
{
  const auto v =
      _rc_vel_provider.getVelocity(_velocity_interp_method, *_face_info, determineState(), _tid);
  const auto rho_face = _rho(makeFace(*_face_info,
                                      limiterType(_advected_interp_method),
                                      MetaPhysicL::raw_value(v) * _normal > 0),
                             determineState());
  return _normal * v * rho_face;
}

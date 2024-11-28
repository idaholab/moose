//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVInletIntensityTKEBC.h"

registerMooseObject("NavierStokesApp", INSFVInletIntensityTKEBC);

InputParameters
INSFVInletIntensityTKEBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Adds inlet boudnary conditon for the turbulent kinetic energy based "
                             "on turbulent intensity.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("intensity", "Turbulent intensity.");
  return params;
}

INSFVInletIntensityTKEBC::INSFVInletIntensityTKEBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    _u(getFunctor<ADReal>("u")),
    _v(isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _w(isParamValid("w") ? &getFunctor<ADReal>("w") : nullptr),
    _intensity(getFunctor<ADReal>("intensity")),
    _dim(_subproblem.mesh().dimension())
{
  if (_dim >= 2 && !_v)
    mooseError(
        "In two or more dimensions, the v velocity must be supplied using the 'v' parameter");
  if (_dim >= 3 && !_w)
    mooseError("In threedimensions, the w velocity must be supplied using the 'w' parameter");
}

ADReal
INSFVInletIntensityTKEBC::boundaryValue(const FaceInfo & fi, const Moose::StateArg & state) const
{
  const auto boundary_face = singleSidedFaceArg(&fi);

  ADRealVectorValue velocity(_u(boundary_face, state));
  if (_v)
    velocity(1) = (*_v)(boundary_face, state);
  if (_w)
    velocity(2) = (*_w)(boundary_face, state);

  const auto velocity_normal = fi.normal() * velocity;

  return 1.5 * Utility::pow<2>(_intensity(boundary_face, state) * velocity_normal);
}

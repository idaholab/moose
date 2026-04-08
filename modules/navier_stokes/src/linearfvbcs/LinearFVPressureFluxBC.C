//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPressureFluxBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVPressureFluxBC);

InputParameters
LinearFVPressureFluxBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription(
      "Adds a fixed diffusive flux BC which can be used for the assembly of linear "
      "finite volume system and whose normal face gradient values are determined "
      "using the H/A flux and a prescribed boundary velocity. This kernel is only designed "
      "to work with advection-diffusion problems.");
  params.addRequiredParam<MooseFunctorName>("HbyA_flux", "The total HbyA face flux value.");
  params.addRequiredParam<MooseFunctorName>(
      "Ainv", "The 1/A where A is the momentum system diagonal vector.");
  params.addRequiredParam<MooseFunctorName>("u", "The x-velocity functor on the boundary.");
  params.addParam<MooseFunctorName>("v", "The y-velocity functor on the boundary.");
  params.addParam<MooseFunctorName>("w", "The z-velocity functor on the boundary.");
  params.addRequiredParam<MooseFunctorName>(
      NS::density, "The density functor used together with the prescribed boundary velocity.");
  return params;
}

LinearFVPressureFluxBC::LinearFVPressureFluxBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _HbyA_flux(getFunctor<Real>("HbyA_flux")),
    _Ainv(getFunctor<RealVectorValue>("Ainv")),
    _dim(_subproblem.mesh().dimension()),
    _u(getFunctor<Real>("u")),
    _v(parameters.isParamValid("v") ? &getFunctor<Real>("v") : nullptr),
    _w(parameters.isParamValid("w") ? &getFunctor<Real>("w") : nullptr),
    _rho(getFunctor<Real>(NS::density))
{
  if (_dim >= 2 && !_v)
    paramError("v", "The 'v' boundary velocity functor must be provided for 2D and 3D problems.");

  if (_dim >= 3 && !_w)
    paramError("w", "The 'w' boundary velocity functor must be provided for 3D problems.");

  if (_dim < 2 && _v)
    paramError("v", "The 'v' boundary velocity functor is only valid in 2D and 3D problems.");

  if (_dim < 3 && _w)
    paramError("w", "The 'w' boundary velocity functor is only valid in 3D problems.");
}

Real
LinearFVPressureFluxBC::computeRequiredPressureFlux() const
{
  const auto face_arg = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  Real required_pressure_flux = _HbyA_flux(face_arg, state);
  const auto & normal = _current_face_info->normal();
  Real boundary_velocity_dot_normal = _u(face_arg, state) * normal(0);

  if (_dim >= 2)
    boundary_velocity_dot_normal += (*_v)(face_arg, state) * normal(1);

  if (_dim >= 3)
    boundary_velocity_dot_normal += (*_w)(face_arg, state) * normal(2);

  required_pressure_flux += _rho(face_arg, state) * boundary_velocity_dot_normal;

  return required_pressure_flux;
}

Real
LinearFVPressureFluxBC::computeBoundaryAinv() const
{
  const auto face_arg = singleSidedFaceArg(_current_face_info);
  const auto face_ainv = _Ainv(face_arg, determineState());
  const auto & normal = _current_face_info->normal();

  // Match the boundary-normal coefficient used by LinearFVAnisotropicDiffusion:
  // for a diagonal tensor Ainv, the effective normal coefficient is n^T Ainv n.
  Real normal_ainv = 0.0;
  for (const auto i : make_range(_dim))
    normal_ainv += normal(i) * normal(i) * face_ainv(i);

  return std::max(face_ainv(0), 1e-8);
  // return std::max(normal_ainv, 1e-8);
}

Real
LinearFVPressureFluxBC::computeBoundaryValue() const
{
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();
  const Real distance = computeCellToFaceDistance();

  return _var.getElemValue(*elem_info, determineState()) -
         computeRequiredPressureFlux() / computeBoundaryAinv() * distance;
}

Real
LinearFVPressureFluxBC::computeBoundaryNormalGradient() const
{
  return -computeRequiredPressureFlux() / computeBoundaryAinv();
}

Real
LinearFVPressureFluxBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVPressureFluxBC::computeBoundaryValueRHSContribution() const
{
  const Real distance = computeCellToFaceDistance();

  return -computeRequiredPressureFlux() / computeBoundaryAinv() * distance;
}

Real
LinearFVPressureFluxBC::computeBoundaryGradientMatrixContribution() const
{
  return 0.0;
}

Real
LinearFVPressureFluxBC::computeBoundaryGradientRHSContribution() const
{
  return -computeRequiredPressureFlux();
}

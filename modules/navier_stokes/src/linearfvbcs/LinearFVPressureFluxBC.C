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
      "using the H/A flux and a prescribed boundary velocity. This boundary condition is only "
      "designed to work with advection-diffusion problems.");
  params.addRequiredParam<MooseFunctorName>("HbyA_flux", "The total HbyA face flux value.");
  params.addRequiredParam<MooseFunctorName>(
      "Ainv", "The 1/A where A is the momentum system diagonal vector.");
  params.addParam<bool>(
      "use_two_term_expansion",
      true,
      "Whether to reconstruct the boundary pressure using the pressure flux and cell gradient. If "
      "false, the boundary pressure is approximated by the adjacent cell pressure.");
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
    _two_term_expansion(getParam<bool>("use_two_term_expansion")),
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

  if (_two_term_expansion)
    _var.computeCellGradients();
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

  // FaceInfo normals point from element to neighbor, so reverse the prescribed velocity flux when
  // this boundary condition acts on the neighbor side of an internal face.
  const Real boundary_normal_multiplier =
      _current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0;
  required_pressure_flux +=
      _rho(face_arg, state) * boundary_normal_multiplier * boundary_velocity_dot_normal;

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

  if (normal_ainv < 0.0)
    mooseError("The boundary-normal Ainv coefficient must be nonnegative, but its value is ",
               normal_ainv,
               ".");

  return normal_ainv;
}

Real
LinearFVPressureFluxBC::computeBoundaryValue() const
{
  const auto state = determineState();
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  if (!_two_term_expansion)
    return _var.getElemValue(*elem_info, state);

  const Real normal_ainv = computeBoundaryAinv();

  // Ainv is initialized to zero and is first populated by the momentum assembly. Until then, use
  // the cell pressure (1 term expansion) for the initial pressure-gradient
  // calculation.
  if (normal_ainv == 0.0)
    return _var.getElemValue(*elem_info, state);

  const Real distance = computeCellToFaceDistance();
  const auto d_cf = computeCellToFaceVector();
  const auto & face_normal = _current_face_info->normal();
  const auto tangential_cell_to_face = d_cf - (d_cf * face_normal) * face_normal;

  // Return the 2-term expansion for the boundary value
  return _var.getElemValue(*elem_info, state) + computeBoundaryNormalGradient() * distance +
         _var.gradSln(*elem_info, state) * tangential_cell_to_face;
}

Real
LinearFVPressureFluxBC::computeBoundaryNormalGradient() const
{
  if (!_two_term_expansion)
    return 0.0;

  const Real normal_ainv = computeBoundaryAinv();
  if (normal_ainv == 0.0)
    return 0.0;

  const auto state = determineState();
  const auto face_ainv = _Ainv(singleSidedFaceArg(_current_face_info), state);
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();
  const Real boundary_normal_multiplier =
      _current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0;
  const auto boundary_normal = boundary_normal_multiplier * _current_face_info->normal();

  RealVectorValue normal_scaled_ainv;
  for (const auto i : make_range(_dim))
    normal_scaled_ainv(i) = boundary_normal(i) * face_ainv(i);

  // The prescribed pressure flux is the complete tensor-weighted flux. Subtract its tangential
  // part only when inverting that flux to reconstruct the boundary-normal pressure gradient.
  const auto tangential_ainv = normal_scaled_ainv - normal_ainv * boundary_normal;
  const Real tangential_pressure_flux = tangential_ainv * _var.gradSln(*elem_info, state);

  return (-computeRequiredPressureFlux() - tangential_pressure_flux) / normal_ainv;
}

Real
LinearFVPressureFluxBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVPressureFluxBC::computeBoundaryValueRHSContribution() const
{
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();
  return computeBoundaryValue() - _var.getElemValue(*elem_info, determineState());
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

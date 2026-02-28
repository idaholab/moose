//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVFunctorRadiativeBC.h"
#include "MathUtils.h"

registerMooseObject("HeatTransferApp", LinearFVFunctorRadiativeBC);

InputParameters
LinearFVFunctorRadiativeBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription(
      "Boundary condition for radiative heat flux in a linear finite volume system. "
      "The nonlinear radiative flux q = sigma * emissivity * (T^4 - Tinfinity^4) is "
      "Newton-linearized around the current cell-center temperature at each assembly "
      "step, providing an implicit matrix contribution and an explicit RHS contribution. "
      "Convergence to the nonlinear solution is achieved through Picard iteration driven "
      "by transient stepping or a coupled nonlinear solve.");
  params.addRequiredParam<MooseFunctorName>(
      "emissivity", "Functor describing the surface emissivity for the radiative boundary condition");
  params.addRequiredParam<MooseFunctorName>(
      "Tinfinity", "Functor for the far-field temperature of the body in radiative heat transfer");
  params.addParam<Real>(
      "stefan_boltzmann_constant", 5.670374419e-8, "The Stefan-Boltzmann constant");
  params.addParam<MooseFunctorName>(
      "diffusion_coeff",
      1.0,
      "Functor for the thermal conductivity, used in boundary value and normal gradient "
      "extrapolations. Should match the diffusion coefficient used in LinearFVDiffusion.");
  return params;
}

LinearFVFunctorRadiativeBC::LinearFVFunctorRadiativeBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _emissivity(getFunctor<Real>("emissivity")),
    _tinf(getFunctor<Real>("Tinfinity")),
    _sigma(getParam<Real>("stefan_boltzmann_constant")),
    _diffusion_coeff(getFunctor<Real>("diffusion_coeff"))
{
  _var.computeCellGradients();
}

Real
LinearFVFunctorRadiativeBC::computeBoundaryGradientMatrixContribution() const
{
  // Newton linearization of q_out = sigma*eps*(T^4 - Tinf^4) around T_old:
  //   q_out ≈ [4*sigma*eps*T_old^3]*T - [sigma*eps*(3*T_old^4 + Tinf^4)]
  // Implicit coefficient: 4*sigma*eps*T_old^3  (always positive → diagonal-dominant)
  const auto & elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto state = determineState();
  const auto T_old = _var.getElemValue(*elem_info, state);
  const auto face = singleSidedFaceArg(_current_face_info);

  return 4.0 * _sigma * _emissivity(face, state) * Utility::pow<3>(T_old);
}

Real
LinearFVFunctorRadiativeBC::computeBoundaryGradientRHSContribution() const
{
  // Explicit part of the Newton linearization: sigma*eps*(3*T_old^4 + Tinf^4)
  // Verified: rhs - matrix*T_old = sigma*eps*(3*T_old^4+Tinf^4) - 4*sigma*eps*T_old^4
  //                              = -sigma*eps*(T_old^4 - Tinf^4) = -q_out  ✓
  const auto & elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto state = determineState();
  const auto T_old = _var.getElemValue(*elem_info, state);
  const auto face = singleSidedFaceArg(_current_face_info);

  return _sigma * _emissivity(face, state) *
         (3.0 * Utility::pow<4>(T_old) + Utility::pow<4>(_tinf(face, state)));
}

Real
LinearFVFunctorRadiativeBC::computeBoundaryNormalGradient() const
{
  // Normal gradient (outward): dT/dn = -q_out / k = -sigma*eps*(T_old^4 - Tinf^4) / k
  const auto & elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto state = determineState();
  const auto T_old = _var.getElemValue(*elem_info, state);
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto face_cd = makeCDFace(*_current_face_info);

  return -_sigma * _emissivity(face, state) *
         (Utility::pow<4>(T_old) - Utility::pow<4>(_tinf(face, state))) /
         _diffusion_coeff(face_cd, state);
}

Real
LinearFVFunctorRadiativeBC::computeBoundaryValue() const
{
  // Extrapolate boundary temperature: T_b = T_P + (dT/dn)*d + grad_correction
  const auto & elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto state = determineState();
  const Real distance = computeCellToFaceDistance();
  const auto d_cf = computeCellToFaceVector();
  const auto correction_vector =
      d_cf - (d_cf * _current_face_info->normal()) * _current_face_info->normal();

  return _var.getElemValue(*elem_info, state) + computeBoundaryNormalGradient() * distance +
         _var.gradSln(*elem_info) * correction_vector;
}

Real
LinearFVFunctorRadiativeBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVFunctorRadiativeBC::computeBoundaryValueRHSContribution() const
{
  // RHS contribution = boundary_value - T_P = (dT/dn)*d + grad_correction
  const auto & elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const Real distance = computeCellToFaceDistance();
  const auto d_cf = computeCellToFaceVector();
  const auto correction_vector =
      d_cf - (d_cf * _current_face_info->normal()) * _current_face_info->normal();

  return computeBoundaryNormalGradient() * distance + _var.gradSln(*elem_info) * correction_vector;
}

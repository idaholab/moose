//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFV2PMomentumAdvectionSlip.h"
#include "NS.h"
#include "FVUtils.h"
#include "INSFVRhieChowInterpolator.h"
#include "SystemBase.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", WCNSFV2PMomentumAdvectionSlip);

InputParameters
WCNSFV2PMomentumAdvectionSlip::validParams()
{
  InputParameters params = INSFVMomentumAdvection::validParams();
  params.addClassDescription(
      "Computes the slip velocity advection kernel for two-phase mixture model.");
  params.addRequiredParam<MooseFunctorName>("u_slip", "The slip velocity in the x direction.");
  params.addParam<MooseFunctorName>("v_slip", "The slip velocity in the y direction.");
  params.addParam<MooseFunctorName>("w_slip", "The slip velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("rho_d", "Dispersed phase density.");
  params.addParam<MooseFunctorName>("fd", 0.0, "Fraction dispersed phase.");
  params.renameParam("fd", "fraction_dispersed", "");
  params.setDocString(NS::density, "Main phase (not dispersed) density functor");

  return params;
}

WCNSFV2PMomentumAdvectionSlip::WCNSFV2PMomentumAdvectionSlip(const InputParameters & params)
  : INSFVMomentumAdvection(params),
    _rho_d(getFunctor<ADReal>("rho_d")),
    _dim(_subproblem.mesh().dimension()),
    _u_slip(getFunctor<ADReal>("u_slip")),
    _v_slip(isParamValid("v_slip") ? &getFunctor<ADReal>("v_slip") : nullptr),
    _w_slip(isParamValid("w_slip") ? &getFunctor<ADReal>("w_slip") : nullptr),
    _fd(getFunctor<ADReal>("fd"))
{
  if (_dim >= 2 && !_v_slip)
    mooseError("In two or more dimensions, the v_slip velocity must be supplied using the 'v_slip' "
               "parameter");
  if (_dim >= 3 && !_w_slip)
    mooseError(
        "In three dimensions, the w_slip velocity must be supplied using the 'w_slip' parameter");
}

void
WCNSFV2PMomentumAdvectionSlip::computeResidualsAndAData(const FaceInfo & fi)
{
  mooseAssert(!skipForBoundary(fi),
              "We shouldn't get in here if we're supposed to skip for a boundary");

  constexpr Real offset = 1e-15;
  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(std::make_pair(_var.number(), _var.sys().number()));
  const auto state = determineState();

  using namespace Moose::FV;

  const bool correct_skewness = _advected_interp_method == InterpMethod::SkewCorrectedAverage;
  const bool on_boundary = onBoundary(fi);

  _elem_residual = 0, _neighbor_residual = 0, _ae = 0, _an = 0;

  Moose::FaceArg face_arg;
  if (on_boundary)
    face_arg = singleSidedFaceArg();
  else
    face_arg = Moose::FaceArg{
        &fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr};

  ADRealVectorValue u_slip_vel_vec;
  if (_dim == 1)
    u_slip_vel_vec(0) = _u_slip(face_arg, state);
  if (_dim == 2)
    u_slip_vel_vec = ADRealVectorValue(_u_slip(face_arg, state), (*_v_slip)(face_arg, state), 0.0);
  if (_dim == 3)
    u_slip_vel_vec = ADRealVectorValue(
        _u_slip(face_arg, state), (*_v_slip)(face_arg, state), (*_w_slip)(face_arg, state));

  const auto rho_mix = _rho(face_arg, state) +
                       (_rho_d(face_arg, state) - _rho(face_arg, state)) * _fd(face_arg, state);

  const auto vdotn = _normal * u_slip_vel_vec;

  if (on_boundary)
  {
    const auto ssf = singleSidedFaceArg();
    const Elem * const sided_elem = ssf.face_side;
    const auto dof_number = sided_elem->dof_number(_sys.number(), _var.number(), 0);
    const auto rho_face = rho_mix;
    const auto eps_face = epsilon()(ssf, state);
    const auto u_face = _var(ssf, state);
    const Real d_u_face_d_dof = u_face.derivatives()[dof_number];
    const auto coeff = vdotn * rho_face / eps_face;

    if (sided_elem == fi.elemPtr())
    {
      _ae = coeff * d_u_face_d_dof;
      _elem_residual = coeff * u_face;
      if (_approximate_as)
        _ae = _cs / fi.elem().n_sides() * rho_face / eps_face;
    }
    else
    {
      _an = -coeff * d_u_face_d_dof;
      _neighbor_residual = -coeff * u_face;
      if (_approximate_as)
        _an = _cs / fi.neighborPtr()->n_sides() * rho_face / eps_face;
    }
  }
  else // we are an internal fluid flow face
  {
    const bool elem_is_upwind = MetaPhysicL::raw_value(u_slip_vel_vec) * _normal > 0;
    const Moose::FaceArg advected_face_arg{&fi,
                                           limiterType(_advected_interp_method),
                                           elem_is_upwind,
                                           correct_skewness,
                                           nullptr,
                                           nullptr};
    if (const auto [is_jump, eps_elem_face, eps_neighbor_face] =
            NS::isPorosityJumpFace(epsilon(), fi, state);
        is_jump)
    {
      // For a weakly compressible formulation, the density should not depend on pressure and
      // consequently the density should not be impacted by the pressure jump that occurs at a
      // porosity jump. Consequently we will allow evaluation of the density using both upstream and
      // downstream information
      const auto rho_face =
          _rho_d(advected_face_arg, state) - _rho(advected_face_arg, state) + offset;

      // We set the + and - sides of the superficial velocity equal to the interpolated value
      const auto & var_elem_face = u_slip_vel_vec(_index);
      const auto & var_neighbor_face = u_slip_vel_vec(_index);

      const auto elem_dof_number = fi.elem().dof_number(_sys.number(), _var.number(), 0);
      const auto neighbor_dof_number = fi.neighbor().dof_number(_sys.number(), _var.number(), 0);

      const auto d_var_elem_face_d_elem_dof = var_elem_face.derivatives()[elem_dof_number];
      const auto d_var_neighbor_face_d_neighbor_dof =
          var_neighbor_face.derivatives()[neighbor_dof_number];

      // We allow a discontintuity in the advective momentum flux at the jump face. Mainly the
      // advective flux is:
      // elem side:
      // rho * v_superficial / eps_elem * v_superficial = rho * v_interstitial_elem * v_superficial
      // neighbor side:
      // rho * v_superficial / eps_neigh * v_superficial = rho * v_interstitial_neigh *
      // v_superficial
      const auto elem_coeff = vdotn * rho_face / eps_elem_face;
      const auto neighbor_coeff = vdotn * rho_face / eps_neighbor_face;
      _ae = elem_coeff * d_var_elem_face_d_elem_dof;
      _elem_residual = elem_coeff * var_elem_face;
      _an = -neighbor_coeff * d_var_neighbor_face_d_neighbor_dof;
      _neighbor_residual = -neighbor_coeff * var_neighbor_face;
      if (_approximate_as)
      {
        _ae = _cs / fi.elem().n_sides() * rho_face / eps_elem_face;
        _an = _cs / fi.neighborPtr()->n_sides() * rho_face / eps_elem_face;
      }
    }
    else
    {
      const auto [interp_coeffs, advected] =
          interpCoeffsAndAdvected(*_rho_u, advected_face_arg, state);

      const auto elem_arg = elemArg();
      const auto neighbor_arg = neighborArg();

      const auto rho_elem = _rho_d(elem_arg, state) - _rho(elem_arg, state) + offset;
      const auto rho_neighbor = _rho_d(neighbor_arg, state) - _rho(neighbor_arg, state) + offset;
      const auto eps_elem = epsilon()(elem_arg, state),
                 eps_neighbor = epsilon()(neighbor_arg, state);
      const auto var_elem = advected.first / rho_elem * eps_elem,
                 var_neighbor = advected.second / rho_neighbor * eps_neighbor;

      _ae = vdotn * rho_elem / eps_elem * interp_coeffs.first;
      // Minus sign because we apply a minus sign to the residual in computeResidual
      _an = -vdotn * rho_neighbor / eps_neighbor * interp_coeffs.second;

      _elem_residual = _ae * var_elem - _an * var_neighbor;
      _neighbor_residual = -_elem_residual;

      if (_approximate_as)
      {
        _ae = _cs / fi.elem().n_sides() * rho_elem / eps_elem;
        _an = _cs / fi.neighborPtr()->n_sides() * rho_neighbor / eps_neighbor;
      }
    }
  }

  _ae *= fi.faceArea() * fi.faceCoord();
  _an *= fi.faceArea() * fi.faceCoord();
  _elem_residual *= fi.faceArea() * fi.faceCoord();
  _neighbor_residual *= fi.faceArea() * fi.faceCoord();
}

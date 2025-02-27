//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearWCNSFV2PMomentumDriftFlux.h"
#include "NS.h"
#include "RhieChowMassFlux.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearFVAdvectionDiffusionBC.h"

registerMooseObject("NavierStokesApp", LinearWCNSFV2PMomentumDriftFlux);

InputParameters
LinearWCNSFV2PMomentumDriftFlux ::validParams()
{
  auto params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Implements the drift momentum flux source.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object",
      "The rhie-chow user-object which is used to determine the face velocity.");
  params.addRequiredParam<MooseFunctorName>("u_slip", "The slip velocity in the x direction.");
  params.addParam<MooseFunctorName>("v_slip", "The slip velocity in the y direction.");
  params.addParam<MooseFunctorName>("w_slip", "The slip velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("rho_d", "Dispersed phase density.");
  params.addParam<MooseFunctorName>("fd", 0.0, "Fraction dispersed phase.");
  params.renameParam("fd", "fraction_dispersed", "");

  params.addParam<bool>(
      "force_boundary_execution", true, "This kernel should execute on boundaries by default");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");

  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>("density_interp_method",
                             coeff_interp_method,
                             "Switch that can select face interpolation method for the density.");

  return params;
}

LinearWCNSFV2PMomentumDriftFlux ::LinearWCNSFV2PMomentumDriftFlux(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _mass_flux_provider(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _rho_d(getFunctor<Real>("rho_d")),
    _f_d(getFunctor<Real>("fd")),
    _u_slip(getFunctor<Real>("u_slip")),
    _v_slip(isParamValid("v_slip") ? &getFunctor<Real>("v_slip") : nullptr),
    _w_slip(isParamValid("w_slip") ? &getFunctor<Real>("w_slip") : nullptr),
    _index(getParam<MooseEnum>("momentum_component")),
    _density_interp_method(
        Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("density_interp_method")))
{
  if (_dim >= 2 && !_v_slip)
    mooseError("In two or more dimensions, the v_slip velocity must be supplied using the 'v_slip' "
               "parameter");
  if (_dim >= 3 && !_w_slip)
    mooseError(
        "In three dimensions, the w_slip velocity must be supplied using the 'w_slip' parameter");

  // Phase fraction could be a nonlinear variable
  const auto & fraction_name = getParam<MooseFunctorName>("fraction_dispersed");
  if (isParamValid("fraction_dispersed") && _fe_problem.hasVariable(fraction_name))
    addMooseVariableDependency(&_fe_problem.getVariable(_tid, fraction_name));
}

void
LinearWCNSFV2PMomentumDriftFlux::computeFlux()
{
  const auto normal = _current_face_info->normal();
  const auto state = determineState();
  const bool on_boundary = Moose::FV::onBoundary(*this, *_current_face_info);

  Moose::FaceArg face_arg;
  if (on_boundary)
    face_arg = singleSidedFaceArg(_current_face_info);
  else
    face_arg = makeCDFace(*_current_face_info);

  RealVectorValue u_slip_vel_vec;
  if (_dim == 1)
    u_slip_vel_vec = RealVectorValue(_u_slip(face_arg, state), 0.0, 0.0);
  else if (_dim == 2)
    u_slip_vel_vec = RealVectorValue(_u_slip(face_arg, state), (*_v_slip)(face_arg, state), 0.0);
  else
    u_slip_vel_vec = RealVectorValue(
        _u_slip(face_arg, state), (*_v_slip)(face_arg, state), (*_w_slip)(face_arg, state));

  const auto uslipdotn = normal * u_slip_vel_vec;

  Real face_rho_fd;
  if (on_boundary)
    face_rho_fd = _rho_d(face_arg, state) * _f_d(face_arg, state);
  else
  {
    const auto elem_arg = makeElemArg(_current_face_info->elemPtr());
    const auto neigh_arg = makeElemArg(_current_face_info->neighborPtr());

    Moose::FV::interpolate(_density_interp_method,
                           face_rho_fd,
                           _rho_d(elem_arg, state) * _f_d(elem_arg, state),
                           _rho_d(neigh_arg, state) * _f_d(neigh_arg, state),
                           *_current_face_info,
                           true);
  }

  _face_flux = -face_rho_fd * uslipdotn * u_slip_vel_vec(_index);
}

Real
LinearWCNSFV2PMomentumDriftFlux::computeElemMatrixContribution()
{
  const auto u_old = _var(makeCDFace(*_current_face_info), Moose::previousNonlinearState()).value();
  if (std::abs(u_old) > 1e-6)
    return _velocity_interp_coeffs.first * _face_flux / u_old * _current_face_area;
  else
    return 0.;
}

Real
LinearWCNSFV2PMomentumDriftFlux::computeNeighborMatrixContribution()
{
  const auto u_old = _var(makeCDFace(*_current_face_info), Moose::previousNonlinearState()).value();
  if (std::abs(u_old) > 1e-6)
    return _velocity_interp_coeffs.second * _face_flux / u_old * _current_face_area;
  else
    // place term on RHS if u_old is too close to 0
    return 0.;
}

Real
LinearWCNSFV2PMomentumDriftFlux::computeElemRightHandSideContribution()
{
  // Get old velocity
  const auto u_old = _var(makeCDFace(*_current_face_info), Moose::previousNonlinearState()).value();
  const auto u = _var(makeCDFace(*_current_face_info), Moose::currentState()).value();
  if (std::abs(u_old) > 1e-6)
    return _velocity_interp_coeffs.first * _face_flux * (u / u_old - 1) * _current_face_area;
  else
    return -_face_flux * _current_face_area;
}

Real
LinearWCNSFV2PMomentumDriftFlux::computeNeighborRightHandSideContribution()
{
  // Get old velocity
  const auto u_old = _var(makeCDFace(*_current_face_info), Moose::previousNonlinearState()).value();
  const auto u = _var(makeCDFace(*_current_face_info), Moose::currentState()).value();
  if (std::abs(u_old) > 1e-6)
    return _velocity_interp_coeffs.second * _face_flux * (u / u_old - 1) * _current_face_area;
  else
    return -_face_flux * _current_face_area;
}

Real
LinearWCNSFV2PMomentumDriftFlux::computeBoundaryRHSContribution(
    const LinearFVBoundaryCondition & /*bc*/)
{
  // Lagging the whole term for now
  // TODO: make sure this only gets called once, and not once per BC
  return -_face_flux * _current_face_area;
}

void
LinearWCNSFV2PMomentumDriftFlux::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);

  // Caching the mass flux on the face which will be reused in the advection term's matrix and right
  // hand side contributions
  computeFlux();

  // Caching the interpolation coefficients so they will be reused for the matrix and right hand
  // side terms
  // Note: We should use Rhie Chow coefficients here, because the way we split the flux is:
  // phi = u * (Matrix term) phi / u_old + (RHS) phi (u / u_old - 1)
  // This will be a fine approximation for now
  _velocity_interp_coeffs =
      Moose::FV::interpCoeffs(_density_interp_method,
                              *_current_face_info,
                              true,
                              _mass_flux_provider.getMassFlux(*_current_face_info));
}

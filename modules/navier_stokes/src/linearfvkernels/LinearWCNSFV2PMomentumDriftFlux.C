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
#include "NavierStokesMethods.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearFVAdvectionDiffusionBC.h"

#include <limits>

registerMooseObject("NavierStokesApp", LinearWCNSFV2PMomentumDriftFlux);

InputParameters
LinearWCNSFV2PMomentumDriftFlux ::validParams()
{
  auto params = LinearWCNSFV2PDriftFluxBase::validParams();
  params.addClassDescription(
      "Implements the diffusion (drift) stress of the two-phase mixture model, "
      "div(beta_d beta_c / rho_m * u_slip (x) u_slip), on the left hand side of the mixture "
      "momentum equation.");
  // The stress is assembled on every face this kernel visits; whether it should be withheld on
  // impermeable walls, as the phase and energy drift fluxes are, is left as it was.
  params.suppressParameter<std::vector<BoundaryName>>("slip_boundaries");
  // Nothing in this kernel reads the Rhie-Chow object: the flux it assembles is carried by the
  // slip velocity, not by the mixture mass flux. The parameter is kept so that existing inputs and
  // the Physics, which set it on every momentum flux kernel alike, keep parsing.
  params.addParam<UserObjectName>("rhie_chow_user_object",
                                  "Unused by this kernel, accepted for input compatibility.");
  params.addRequiredParam<MooseFunctorName>("rho_c", "Continuous phase density.");

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
  : LinearWCNSFV2PDriftFluxBase(params),
    _rho_c(getFunctor<Real>("rho_c")),
    _index(getParam<MooseEnum>("momentum_component")),
    _density_interp_method(
        Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("density_interp_method"))),
    _face_flux(0.0),
    _slip_mass_flux(0.0),
    _gamma(0.0)
{
  // The deferred correction of computeCoefficients() and deferredCorrection() reads this
  // variable at the previous outer iteration, so that state has to exist. Requesting it here
  // rather than leaving the input to set 'previous_nl_solution_required' keeps the requirement
  // with the object that has it, and allocates the vector before assembly rather than lazily
  // inside a threaded loop, which is not allowed.
  _var.sys().needSolutionState(1, Moose::SolutionIterationType::Nonlinear);

  // Note that since this is used in a segregated solver at this time, we don't need to declare
  // that this kernel may depend on a phase fraction variable
}

void
LinearWCNSFV2PMomentumDriftFlux::computeFlux()
{
  const auto & normal = _current_face_info->normal();
  const auto state = determineState();
  const auto u_slip_vel_vec = slipVelocity(currentFaceArg(), state);
  const auto uslipdotn = normal * u_slip_vel_vec;

  // The exact diffusion stress coefficient, beta_d beta_c / rho_m, evaluated on the face
  const auto face_coefficient = faceCoefficient([this](const auto & arg, const auto & state)
                                                { return diffusionStressCoefficient(arg, state); },
                                                state,
                                                _density_interp_method);

  // The term is written as a flux carried by the slip velocity, so that the flux scale can be
  // reused below as the scale of the implicit surrogate.
  //
  // Sign. A LinearFVFluxKernel assembles its face flux onto the left hand side, so the flux set
  // here is the left hand side form of the term. Summing the phase momentum equations puts
  // +div(sum_k a_k rho_k u_Mk u_Mk) on the left hand side, hence the positive sign. This differs
  // from the nonlinear WCNSFV2PMomentumDriftFlux, which carries the opposite sign and is left
  // unchanged; the two discretizations therefore disagree on this term by construction.
  _slip_mass_flux = face_coefficient * uslipdotn;
  _face_flux = _slip_mass_flux * u_slip_vel_vec(_index);
}

Real
LinearWCNSFV2PMomentumDriftFlux::computeElemMatrixContribution()
{
  return std::max(_gamma, 0.0) * _current_face_area;
}

Real
LinearWCNSFV2PMomentumDriftFlux::computeNeighborMatrixContribution()
{
  return -std::max(-_gamma, 0.0) * _current_face_area;
}

Real
LinearWCNSFV2PMomentumDriftFlux::deferredCorrection() const
{
  // Upwind value of the previous iterate with respect to the slip mass flux
  const auto old_state = Moose::previousNonlinearState();
  const auto u_upwind_old =
      _gamma > 0 ? _var(makeElemArg(_current_face_info->elemPtr()), old_state).value()
                 : _var(makeElemArg(_current_face_info->neighborPtr()), old_state).value();

  return (_gamma * u_upwind_old - _face_flux) * _current_face_area;
}

Real
LinearWCNSFV2PMomentumDriftFlux::computeElemRightHandSideContribution()
{
  return deferredCorrection();
}

Real
LinearWCNSFV2PMomentumDriftFlux::computeNeighborRightHandSideContribution()
{
  // The right hand side contributions are not negated by the assembly routine, unlike the matrix
  // ones, so the neighbour row has to be given the opposite sign here
  return -deferredCorrection();
}

Real
LinearWCNSFV2PMomentumDriftFlux::computeBoundaryRHSContribution(
    const LinearFVBoundaryCondition & /*bc*/)
{
  // Lagging the whole term for now
  // TODO: make sure this only gets called once, and not once per BC
  return -_boundary_normal_factor * _face_flux * _current_face_area;
}

void
LinearWCNSFV2PMomentumDriftFlux::setupFaceData(const FaceInfo * face_info)
{
  LinearWCNSFV2PDriftFluxBase::setupFaceData(face_info);

  // Caching the flux on the face which will be reused in the matrix and right hand side
  // contributions
  computeFlux();

  // Coefficient of the implicit surrogate. This term has no genuine linear dependence on the
  // velocity component being solved for: the slip velocity is driven by the pressure gradient and
  // by gravity through its algebraic closure, not by the local cell value. The matrix contribution
  // is therefore a deferred correction rather than a linearization, and the deferredCorrection()
  // added to the right hand side cancels it exactly at convergence. The converged solution does
  // not depend on _gamma, which is chosen for convergence alone.
  //
  // The natural choice is the Picard ratio flux / u_old, but that grows without bound as u_old
  // approaches zero and carries no controlled sign. We cap its magnitude with the slip mass flux,
  // which is the scale of the term itself, and take the sign of the slip mass flux. Combined with
  // the upwind structure of the matrix contributions this puts a non-negative coefficient on both
  // diagonals and a non-positive one on both off-diagonals, on every face, with no special case
  // near stagnation.
  const auto u_old = _var(makeCDFace(*_current_face_info), Moose::previousNonlinearState()).value();
  const auto ratio = (std::abs(u_old) > libMesh::TOLERANCE) ? std::abs(_face_flux / u_old)
                                                            : std::numeric_limits<Real>::max();
  _gamma = std::copysign(std::min(ratio, std::abs(_slip_mass_flux)), _slip_mass_flux);
}

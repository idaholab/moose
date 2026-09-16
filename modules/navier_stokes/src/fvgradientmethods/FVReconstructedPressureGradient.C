//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVReconstructedPressureGradient.h"

#include "FEProblemBase.h"
#include "FVUtils.h"
#include "LinearFVGradientReader.h"
#include "LinearSystem.h"
#include "MooseMesh.h"
#include "RhieChowMassFlux.h"
#include "SystemBase.h"

#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/elem.h"

#include <cmath>

using namespace libMesh;

registerMooseObject("NavierStokesApp", FVReconstructedPressureGradient);

InputParameters
FVReconstructedPressureGradient::validParams()
{
  InputParameters params = FVGradientMethod::validParams();
  params += MeshChangedInterface::validParams();
  params.suppressParameter<MooseEnum>("limiter");
  params.addClassDescription(
      "Reconstructs and relaxes the pressure gradient used for Rhie-Chow momentum coupling.");
  params.addParam<GradientMethodName>(
      "base_gradient_method",
      "green-gauss",
      "Gradient method used before Rhie-Chow has computed reconstructed gradients.");
  params.addRangeCheckedParam<Real>(
      "gradient_relaxation",
      0.1,
      "0.0<gradient_relaxation<=1.0",
      "Relaxation factor applied when updating the reconstructed pressure-coupling gradient.");
  return params;
}

FVReconstructedPressureGradient::FVReconstructedPressureGradient(const InputParameters & params)
  : FVGradientMethod(params),
    MeshChangedInterface(params),
    _base_gradient_method_name(getParam<GradientMethodName>("base_gradient_method")),
    _gradient_relaxation(getParam<Real>("gradient_relaxation"))
{
}

void
FVReconstructedPressureGradient::linkFlowSystem(RhieChowMassFlux & rc,
                                                const LinearFVGradientReader & pressure_gradient)
{
  mooseAssert(&pressure_gradient.system() == &rc.pressureSystem(),
              "First binding must use the pressure system owned by the RhieChowMassFlux.");

  mooseAssert(pressure_gradient.variableNumber() == rc.pressureVariableNumber(),
              "First binding must use the pressure variable registered on the RhieChowMassFlux.");

  if (!_rhie_chow)
  {
    _rhie_chow = &rc;
    _pressure_system = &pressure_gradient.system();
    _pressure_variable_number = pressure_gradient.variableNumber();
    _momentum_systems.reserve(rc.dimension());
    _velocity_gradient_fields.reserve(rc.dimension());
    for (const auto component : make_range(rc.dimension()))
    {
      _momentum_systems.push_back(&rc.momentumSystem(component));
      _velocity_gradient_fields.push_back(&rc.velocityVariable(component).requestCellGradients());
    }
    return;
  }

  if (_rhie_chow != &rc)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' is already bound to RhieChowMassFlux '",
               _rhie_chow->name(),
               "' and cannot also be used by RhieChowMassFlux '",
               rc.name(),
               "'.");

  if (_pressure_system != &pressure_gradient.system())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' is already bound to pressure system '",
               _pressure_system->name(),
               "' and cannot also be used by pressure system '",
               pressure_gradient.system().name(),
               "'.");

  if (_pressure_variable_number != pressure_gradient.variableNumber())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' is already bound to pressure variable number ",
               _pressure_variable_number,
               " and cannot also be used by pressure variable number ",
               pressure_gradient.variableNumber(),
               ".");

  if (_momentum_systems.size() != rc.dimension())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' cannot be reused with a different set of momentum systems.");

  for (const auto component : make_range(rc.dimension()))
    if (_momentum_systems[component] != &rc.momentumSystem(component))
      mooseError("FVReconstructedPressureGradient '",
                 name(),
                 "' cannot be reused with a different set of momentum systems.");
}

void
FVReconstructedPressureGradient::checkFlowSystem([[maybe_unused]] const RhieChowMassFlux & rc) const
{
  mooseAssert(_rhie_chow,
              "FVReconstructedPressureGradient must be bound to one RhieChowMassFlux before it "
              "is used.");

  mooseAssert(_rhie_chow == &rc,
              "FVReconstructedPressureGradient is bound to a single RhieChowMassFlux and cannot "
              "be used by another.");
}

void
FVReconstructedPressureGradient::validateSetup(const RhieChowMassFlux & rc) const
{
  checkFlowSystem(rc);

  mooseAssert(_pressure_system == &rc.pressureSystem() &&
                  _pressure_variable_number == rc.pressureVariableNumber(),
              "FVReconstructedPressureGradient must be linked to the pressure field owned by its "
              "bound RhieChowMassFlux.");

  const auto & pressure_variable = rc.pressureSystem().getVariable(0, _pressure_variable_number);
  if (pressure_variable.blockIDs() != rc.blockIDs())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' requires pressure variable '",
               pressure_variable.name(),
               "' and RhieChowMassFlux '",
               rc.name(),
               "' to have identical block restrictions. Pressure blocks: ",
               Moose::stringify(pressure_variable.blockIDs()),
               "; Rhie-Chow blocks: ",
               Moose::stringify(rc.blockIDs()),
               ".");

  mooseAssert(_momentum_systems.size() == rc.dimension() &&
                  _velocity_gradient_fields.size() == rc.dimension(),
              "One momentum system and velocity-gradient field must be registered per spatial "
              "component.");

  for (const auto component : make_range(rc.dimension()))
  {
    mooseAssert(_momentum_systems[component] == &rc.momentumSystem(component),
                "Momentum system linkage must match the RhieChowMassFlux for every spatial "
                "component.");

    [[maybe_unused]] const auto * const velocity_gradient = _velocity_gradient_fields[component];
    mooseAssert(velocity_gradient &&
                    &velocity_gradient->system() == &rc.momentumSystem(component) &&
                    velocity_gradient->variableNumber() == rc.velocityVariable(component).number(),
                "Velocity-gradient field linkage must match the RhieChowMassFlux for every "
                "momentum component.");
  }
}

void
FVReconstructedPressureGradient::resolveGradientMethodDependencies(FEProblemBase & fe_problem)
{
  if (_base_gradient_method_name == name())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' cannot use itself as its base_gradient_method.");

  if (_base_gradient_method_name == "green-gauss" &&
      !fe_problem.hasFVGradientMethod(_base_gradient_method_name))
  {
    auto params = fe_problem.getMooseApp().getFactory().getValidParams("FVGreenGaussGradient");
    fe_problem.addFVGradientMethod("FVGreenGaussGradient", _base_gradient_method_name, params);
  }

  if (!fe_problem.hasFVGradientMethod(_base_gradient_method_name))
    mooseError(
        "Unable to find base FVGradientMethod with name '", _base_gradient_method_name, "'.");

  _base_gradient_method = &fe_problem.getFVGradientMethod(_base_gradient_method_name);
  if (_base_gradient_method == this)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' cannot use itself as its base_gradient_method.");
}

void
FVReconstructedPressureGradient::computeGradientWithoutLimiter(
    SystemBase & system,
    GradientContainer & gradient,
    const std::unordered_set<unsigned int> & variable_numbers) const
{
  if (!_pressure_system)
  {
    mooseAssert(_base_gradient_method,
                "resolveGradientMethodDependencies() must run before gradients are computed.");
    _base_gradient_method->computeGradient(system, gradient, variable_numbers);
    return;
  }

  mooseAssert(_pressure_system == &system,
              "FVReconstructedPressureGradient can only compute gradients for the pressure "
              "system it is bound to.");

  mooseAssert(variable_numbers.size() == 1 && variable_numbers.count(_pressure_variable_number),
              "FVReconstructedPressureGradient can only compute the pressure variable to which "
              "it is bound.");

  if (!_coupling_pressure_gradient_initialized)
  {
    // No flux-consistent pressure gradient exists before the first pressure corrector. Use the
    // ordinary gradient for the initial momentum predictor instead of inventing coupling data.
    mooseAssert(_base_gradient_method,
                "resolveGradientMethodDependencies() must run before gradients are computed.");
    _base_gradient_method->computeGradient(system, gradient, variable_numbers);
    return;
  }

  mooseAssert(_coupling_pressure_gradient.size() == gradient.size(),
              "Coupling and destination gradients must have equal component counts.");
  for (const auto component : index_range(gradient))
  {
    mooseAssert(gradient[component]->type() == GHOSTED,
                "Linear FV gradient storage must be ghosted.");
    // Only the owned range needs to be copied here; the caller,
    // FVGradientMethod::computeGradient(), unconditionally closes every component of gradient right
    // after this call returns, which refreshes the ghosts needed by face interpolation from the
    // now-matching owned entries.
    *gradient[component] = *_coupling_pressure_gradient[component];
  }
}

void
FVReconstructedPressureGradient::copyGradient(const GradientView & source,
                                              GradientContainer & destination)
{
  if (destination.empty())
  {
    destination.reserve(source.size());
    for (const auto component : index_range(source))
      destination.push_back(source[component]->clone());
    return;
  }

  mooseAssert(source.size() == destination.size(),
              "Source and destination gradients must have equal component counts.");
  for (const auto component : index_range(source))
    *destination[component] = *source[component];
}

void
FVReconstructedPressureGradient::transition(const PressureGradientReconstructionEvent event)
{
  const auto advance = [this]([[maybe_unused]] const PressureGradientReconstructionState expected,
                              const PressureGradientReconstructionState next)
  {
    mooseAssert(_reconstruction_state == expected,
                "Reconstruction-cycle transition requested from an unexpected state.");
    _reconstruction_state = next;
  };

  switch (event)
  {
    case PressureGradientReconstructionEvent::Reset:
      _reconstruction_state = PressureGradientReconstructionState::NeedLaggedVelocityGradient;
      return;
    case PressureGradientReconstructionEvent::SaveLaggedVelocityGradient:
      advance(PressureGradientReconstructionState::NeedLaggedVelocityGradient,
              PressureGradientReconstructionState::NeedCandidate);
      return;
    case PressureGradientReconstructionEvent::ReconstructCandidate:
      advance(PressureGradientReconstructionState::NeedCandidate,
              PressureGradientReconstructionState::CandidateReady);
      return;
    case PressureGradientReconstructionEvent::PublishCandidate:
      advance(PressureGradientReconstructionState::CandidateReady,
              PressureGradientReconstructionState::NeedLaggedVelocityGradient);
      return;
  }
}

void
FVReconstructedPressureGradient::resetForTimeStep(const RhieChowMassFlux & rc)
{
  checkFlowSystem(rc);
  // A new time-step attempt starts from the last accepted coupling field. This keeps a converged
  // momentum balance unchanged across time-step acceptance, rejection, restart, and recovery.
  copyGradient(rc.pressureGradientField().components(Moose::oldState()),
               _coupling_pressure_gradient);
  _coupling_pressure_gradient_initialized = true;
  transition(PressureGradientReconstructionEvent::Reset);
}

void
FVReconstructedPressureGradient::meshChanged()
{
  _lagged_reconstruction_velocity_gradient.clear();
  _reconstructed_pressure_gradient.clear();
  _coupling_pressure_gradient.clear();
  _coupling_pressure_gradient_initialized = false;
  transition(PressureGradientReconstructionEvent::Reset);
}

void
FVReconstructedPressureGradient::saveLaggedVelocityGradient(RhieChowMassFlux & rc)
{
  checkFlowSystem(rc);
  transition(PressureGradientReconstructionEvent::SaveLaggedVelocityGradient);
  const auto dimension = rc.dimension();

  mooseAssert(_velocity_gradient_fields.size() == dimension,
              "A velocity gradient field must be registered for every momentum component.");

  // Freeze grad(u) before the pressure corrector changes the flux. The lagged field linearizes the
  // Taylor correction from each cell center P to its face f without coupling reconstruction back
  // to the velocity that it is currently computing.
  if (_lagged_reconstruction_velocity_gradient.empty())
    _lagged_reconstruction_velocity_gradient.resize(dimension);

  for (const auto component : make_range(dimension))
    copyGradient(_velocity_gradient_fields[component]->components(),
                 _lagged_reconstruction_velocity_gradient[component]);
}

RealVectorValue
FVReconstructedPressureGradient::reconstructionVelocityGradient(
    const RhieChowMassFlux & rc,
    const ElemInfo & elem_info,
    const FaceInfo & fi,
    const bool elem_has_info,
    const unsigned int velocity_component) const
{
  const auto dimension = rc.dimension();
  const auto & velocity = rc.velocityVariable(velocity_component);
  const auto system_number = rc.momentumSystem(velocity_component).number();

  RealVectorValue elem_gradient;
  for (const auto direction : make_range(dimension))
    elem_gradient(direction) =
        (*_lagged_reconstruction_velocity_gradient[velocity_component][direction])(
            elem_info.dofIndices()[system_number][velocity.number()]);

  const ElemInfo * const neighbor_info = elem_has_info ? fi.neighborInfo() : fi.elemInfo();
  // At a domain boundary or the edge of the Rhie-Chow block restriction, use the owned cell's
  // gradient. Otherwise interpolate the two lagged cell gradients to the face.
  if (!neighbor_info || !rc.hasBlocks(neighbor_info->subdomain_id()))
    return elem_gradient;

  RealVectorValue neighbor_gradient;
  for (const auto direction : make_range(dimension))
    neighbor_gradient(direction) =
        (*_lagged_reconstruction_velocity_gradient[velocity_component][direction])(
            neighbor_info->dofIndices()[system_number][velocity.number()]);

  RealVectorValue face_gradient;
  Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                         face_gradient,
                         elem_gradient,
                         neighbor_gradient,
                         fi,
                         elem_has_info);

  return face_gradient;
}

void
FVReconstructedPressureGradient::assembleFaceProjection(const RhieChowMassFlux & rc,
                                                        const ElemInfo & elem_info,
                                                        const FaceInfo * const fi,
                                                        const Point & surface_vector,
                                                        const bool elem_has_info,
                                                        DenseMatrix<Real> & matrix,
                                                        DenseVector<Real> & projection_rhs) const
{
  mooseAssert(fi, "FaceInfo must be available while reconstructing a cell.");

  const Real surface_area = surface_vector.norm();
  const auto face_normal = surface_vector / surface_area;
  // RhieChow stores the scalar flux relative to FaceInfo::normal(). Flip that orientation when the
  // current cell is on the opposite side so q_f is outward from this cell.
  const Point flux_normal =
      rc.hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->normal() : Point(-fi->normal());
  const Real face_flux = rc.getVolumetricFaceFlux(*fi);
  mooseAssert(std::isfinite(face_flux), "Corrected face flux must be finite.");
  const Real normal_alignment = flux_normal * face_normal;
  Real face_normal_reconstructed_quantity = face_flux * normal_alignment;

  // First-order expansion at the face gives
  //   u_f.n_f = u_P.n_f + ((grad u)_f d_Pf).n_f.
  // Subtract the lagged Taylor term to obtain one face equation for the unknown cell velocity:
  //   u_P.n_f ~= u_f.n_f - ((grad u)_f d_Pf).n_f.
  const Point d_pf = fi->faceCentroid() - elem_info.centroid();
  Real gradient_flux_correction = 0.0;
  for (const auto component : make_range(rc.dimension()))
  {
    const auto velocity_gradient =
        reconstructionVelocityGradient(rc, elem_info, *fi, elem_has_info, component);
    for ([[maybe_unused]] const auto direction : make_range(rc.dimension()))
      mooseAssert(std::isfinite(velocity_gradient(direction)),
                  "Lagged velocity gradient must be finite.");

    gradient_flux_correction += (velocity_gradient * d_pf) * surface_vector(component);
  }

  face_normal_reconstructed_quantity -= gradient_flux_correction / surface_area;

  // Assemble the area-weighted least-squares projection of the corrected face equations:
  //   [sum_f |S_f| n_f n_f^T] u_P = sum_f |S_f| qhat_f n_f,
  // where qhat_f is face_normal_reconstructed_quantity and S_f = |S_f| n_f.
  for (const auto i : make_range(rc.dimension()))
  {
    projection_rhs(i) += face_normal_reconstructed_quantity * surface_vector(i);
    for (const auto j : make_range(rc.dimension()))
      matrix(i, j) += surface_vector(i) * surface_vector(j) / surface_area;
  }
}

DenseVector<Real>
FVReconstructedPressureGradient::solveFaceProjection(const DenseMatrix<Real> & matrix,
                                                     const DenseVector<Real> & projection_rhs) const
{
  const auto dimension = projection_rhs.size();
  DenseVector<Real> reconstructed_quantity(dimension);
  if (dimension == 1)
    reconstructed_quantity(0) = projection_rhs(0) / matrix(0, 0);
  else
  {
    // A valid cell has face normals that span the spatial dimension, making this normal-equation
    // matrix symmetric positive definite.
    DenseMatrix<Real> solve_matrix(matrix);
    solve_matrix.cholesky_solve(projection_rhs, reconstructed_quantity);
  }

  return reconstructed_quantity;
}

Real
FVReconstructedPressureGradient::reconstructPressureGradient(
    const RhieChowMassFlux & rc,
    const ElemInfo & elem_info,
    const unsigned int component,
    const Real reconstructed_velocity) const
{
  mooseAssert(std::isfinite(reconstructed_velocity),
              "Reconstructed velocity component must be finite.");

  const auto momentum_dof = elem_info.dofIndices()[rc.momentumSystem(component).number()][0];
  const Real HbyA = (*rc.HbyAComponents()[component])(momentum_dof);
  const Real Ainv = (*rc.AinvComponents()[component])(momentum_dof);
  mooseAssert(std::isfinite(HbyA) && std::isfinite(Ainv) && Ainv != 0.0,
              "Momentum-coupling H/A and 1/A data must be finite, and 1/A must be nonzero.");

  // Invert the same diagonal momentum relation used by Rhie-Chow,
  //   u_P = -(H/A)_P - A_P^{-1} (grad p)_P,
  // so the reconstructed pressure gradient produces the projected, flux-consistent u_P.
  const Real reconstructed_gradient = (-reconstructed_velocity - HbyA) / Ainv;
  mooseAssert(std::isfinite(reconstructed_gradient),
              "Reconstructed pressure-gradient component must be finite.");

  return reconstructed_gradient;
}

void
FVReconstructedPressureGradient::computeCandidateFromCorrectedFlux(const RhieChowMassFlux & rc)
{
  checkFlowSystem(rc);
  transition(PressureGradientReconstructionEvent::ReconstructCandidate);

  const auto dimension = rc.dimension();
  const auto & base_pressure_gradient = rc.basePressureGradientComponents();
  const auto face_flux_iteration = rc.faceMassFluxGeneration();

  mooseAssert(face_flux_iteration != _last_reconstructed_face_flux_iteration,
              "The current pressure corrector must produce a new face flux before "
              "reconstruction.");

  if (_reconstructed_pressure_gradient.empty())
    for (const auto component : make_range(dimension))
      _reconstructed_pressure_gradient.push_back(base_pressure_gradient[component]->zero_clone());

  // Zero the candidate so pressure DOFs not visited during this corrector cannot retain stale
  // reconstructed values from an earlier cycle.
  for (auto & pressure_gradient : _reconstructed_pressure_gradient)
    pressure_gradient->zero();

  const auto & mesh = rc.pressureSystem().feProblem().mesh();
  const auto rz_radial_coord = mesh.getAxisymmetricRadialCoord();

  for (const auto & elem_info : mesh.elemInfoVector())
  {
    if (!rc.hasBlocks(elem_info->subdomain_id()))
      continue;

    const Elem & elem = *elem_info->elem();
    DenseMatrix<Real> matrix(dimension, dimension);
    DenseVector<Real> projection_rhs(dimension);
    matrix.zero();
    projection_rhs.zero();

    // The coordinate-system-aware surface vector S_f supplied by loopOverElemFaceInfo is outward
    // from this cell, including the appropriate Cartesian or axisymmetric geometric weighting.
    auto act = [&](const Elem &,
                   const Elem * const,
                   const FaceInfo * const fi,
                   const Point & surface_vector,
                   const Real,
                   const bool elem_has_info)
    {
      assembleFaceProjection(
          rc, *elem_info, fi, surface_vector, elem_has_info, matrix, projection_rhs);
    };

    Moose::FV::loopOverElemFaceInfo(
        elem, mesh, act, mesh.getCoordSystem(elem.subdomain_id()), rz_radial_coord);

    const auto reconstructed_quantity = solveFaceProjection(matrix, projection_rhs);

    const auto pressure_dof =
        elem_info->dofIndices()[rc.pressureSystem().number()][rc.pressureVariableNumber()];
    for (const auto component : make_range(dimension))
    {
      const auto reconstructed_gradient =
          reconstructPressureGradient(rc, *elem_info, component, reconstructed_quantity(component));
      _reconstructed_pressure_gradient[component]->set(pressure_dof, reconstructed_gradient);
    }
  }

  for (auto & pressure_gradient : _reconstructed_pressure_gradient)
    pressure_gradient->close();

  _last_reconstructed_face_flux_iteration = face_flux_iteration;
}

const FVReconstructedPressureGradient::GradientContainer &
FVReconstructedPressureGradient::reconstructedCandidate(const RhieChowMassFlux & rc) const
{
  checkFlowSystem(rc);
  mooseAssert(_reconstruction_state == PressureGradientReconstructionState::CandidateReady,
              "No reconstructed pressure-gradient candidate is ready for the conservative "
              "cell-velocity correction.");
  return _reconstructed_pressure_gradient;
}

void
FVReconstructedPressureGradient::updateCouplingPressureGradient(
    const RhieChowMassFlux & rc,
    const GradientView & base_gradient,
    const GradientContainer & reconstructed_candidate)
{
  mooseAssert(!base_gradient.empty() && reconstructed_candidate.size() == base_gradient.size(),
              "Base and reconstructed gradients must have equal nonzero component counts.");

  if (!_coupling_pressure_gradient_initialized)
  {
    copyGradient(base_gradient, _coupling_pressure_gradient);
    _coupling_pressure_gradient_initialized = true;
  }

  const auto & mesh = rc.pressureSystem().feProblem().mesh();
  const auto pressure_system_number = rc.pressureSystem().number();
  for (const auto component : index_range(_coupling_pressure_gradient))
  {
    for (const auto & elem_info : mesh.elemInfoVector())
    {
      if (!rc.hasBlocks(elem_info->subdomain_id()))
        continue;

      const auto pressure_dof =
          elem_info->dofIndices()[pressure_system_number][_pressure_variable_number];
      // Under-relax feedback to the next momentum predictor:
      //   g_coupling^{k+1} = (1-alpha) g_coupling^k + alpha g_reconstructed^k.
      const auto updated_gradient =
          (1.0 - _gradient_relaxation) * (*_coupling_pressure_gradient[component])(pressure_dof) +
          _gradient_relaxation * (*reconstructed_candidate[component])(pressure_dof);
      _coupling_pressure_gradient[component]->set(pressure_dof, updated_gradient);
    }
    _coupling_pressure_gradient[component]->close();
  }
}

void
FVReconstructedPressureGradient::finalizeCouplingPressureGradient(
    const RhieChowMassFlux & rc, const GradientView & base_gradient)
{
  checkFlowSystem(rc);
  transition(PressureGradientReconstructionEvent::PublishCandidate);
  updateCouplingPressureGradient(rc, base_gradient, _reconstructed_pressure_gradient);
}

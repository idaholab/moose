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
  if (&pressure_gradient.system() != &rc.pressureSystem())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' can only be used with pressure system '",
               rc.pressureSystem().name(),
               "' owned by RhieChowMassFlux '",
               rc.name(),
               "', but it was also requested for system '",
               pressure_gradient.system().name(),
               "'.");

  if (pressure_gradient.variableNumber() != rc.pressureVariableNumber())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' can only be used for the pressure variable registered on RhieChowMassFlux '",
               rc.name(),
               "'.");

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
FVReconstructedPressureGradient::checkFlowSystem(const RhieChowMassFlux & rc) const
{
  if (!_rhie_chow)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' must be bound to one RhieChowMassFlux before it is used.");

  if (_rhie_chow != &rc)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' is bound to RhieChowMassFlux '",
               _rhie_chow->name(),
               "' and cannot be used by RhieChowMassFlux '",
               rc.name(),
               "'.");
}

void
FVReconstructedPressureGradient::validateSetup(const RhieChowMassFlux & rc) const
{
  checkFlowSystem(rc);

  if (_pressure_system != &rc.pressureSystem() ||
      _pressure_variable_number != rc.pressureVariableNumber())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' is not linked to the pressure field owned by RhieChowMassFlux '",
               rc.name(),
               "'.");

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

  if (_momentum_systems.size() != rc.dimension() ||
      _velocity_gradient_fields.size() != rc.dimension())
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' requires one momentum system and velocity-gradient field per spatial component "
               "for RhieChowMassFlux '",
               rc.name(),
               "'.");

  for (const auto component : make_range(rc.dimension()))
  {
    if (_momentum_systems[component] != &rc.momentumSystem(component))
      mooseError("FVReconstructedPressureGradient '",
                 name(),
                 "' is linked to an incompatible momentum system for component ",
                 component,
                 " of RhieChowMassFlux '",
                 rc.name(),
                 "'.");

    const auto * const velocity_gradient = _velocity_gradient_fields[component];
    if (!velocity_gradient || &velocity_gradient->system() != &rc.momentumSystem(component) ||
        velocity_gradient->variableNumber() != rc.velocityVariable(component).number())
      mooseError("FVReconstructedPressureGradient '",
                 name(),
                 "' is missing the velocity-gradient field for momentum component ",
                 component,
                 " of RhieChowMassFlux '",
                 rc.name(),
                 "'.");
  }
}

const FVGradientMethod &
FVReconstructedPressureGradient::resolveBaseGradientMethod(SystemBase & system) const
{
  auto & fe_problem = system.feProblem();
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

  const auto & method = fe_problem.getFVGradientMethod(_base_gradient_method_name);
  if (&method == this)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' cannot use itself as its base_gradient_method.");

  return method;
}

void
FVReconstructedPressureGradient::computeGradientWithoutLimiter(
    SystemBase & system,
    GradientContainer & gradient,
    const std::unordered_set<unsigned int> & variable_numbers) const
{
  if (!_pressure_system)
  {
    resolveBaseGradientMethod(system).computeGradient(system, gradient, variable_numbers);
    return;
  }

  if (_pressure_system != &system)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' is bound to pressure system '",
               _pressure_system->name(),
               "' and cannot compute gradients for system '",
               system.name(),
               "'.");

  if (variable_numbers.size() != 1 || !variable_numbers.count(_pressure_variable_number))
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' can only compute the pressure variable to which it is bound.");

  if (!_coupling_pressure_gradient_initialized)
  {
    // No flux-consistent pressure gradient exists before the first pressure corrector. Use the
    // ordinary gradient for the initial momentum predictor instead of inventing coupling data.
    resolveBaseGradientMethod(system).computeGradient(system, gradient, variable_numbers);
    return;
  }

  mooseAssert(_coupling_pressure_gradient.size() == gradient.size(),
              "Coupling and destination gradients must have equal component counts.");
  for (const auto component : index_range(gradient))
  {
    mooseAssert(gradient[component]->type() == GHOSTED,
                "Linear FV gradient storage must be ghosted.");
    // localize() copies owned entries and refreshes ghosts needed by face interpolation on this
    // processor; NumericVector assignment alone does not guarantee updated ghost entries.
    _coupling_pressure_gradient[component]->localize(*gradient[component],
                                                     system.dofMap().get_send_list());
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
FVReconstructedPressureGradient::transition(const ReconstructionEvent event)
{
  const auto state_name = [](const ReconstructionState state)
  {
    switch (state)
    {
      case ReconstructionState::NeedLaggedGradient:
        return "NeedLaggedGradient";
      case ReconstructionState::NeedCandidate:
        return "NeedCandidate";
      case ReconstructionState::CandidateReady:
        return "CandidateReady";
    }
    return "unknown";
  };

  const auto advance = [this, &state_name](const ReconstructionState expected,
                                           const ReconstructionState next,
                                           const char * const action)
  {
    if (_reconstruction_state != expected)
      mooseError("FVReconstructedPressureGradient '",
                 name(),
                 "' cannot ",
                 action,
                 " while its reconstruction state is ",
                 state_name(_reconstruction_state),
                 "; expected ",
                 state_name(expected),
                 ".");
    _reconstruction_state = next;
  };

  switch (event)
  {
    case ReconstructionEvent::Reset:
      _reconstruction_state = ReconstructionState::NeedLaggedGradient;
      return;
    case ReconstructionEvent::SaveLaggedGradient:
      advance(ReconstructionState::NeedLaggedGradient,
              ReconstructionState::NeedCandidate,
              "save a lagged velocity gradient");
      return;
    case ReconstructionEvent::ReconstructCandidate:
      advance(ReconstructionState::NeedCandidate,
              ReconstructionState::CandidateReady,
              "reconstruct a pressure-gradient candidate");
      return;
    case ReconstructionEvent::PublishCandidate:
      advance(ReconstructionState::CandidateReady,
              ReconstructionState::NeedLaggedGradient,
              "publish a pressure-gradient candidate");
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
  transition(ReconstructionEvent::Reset);
}

void
FVReconstructedPressureGradient::meshChanged()
{
  _lagged_reconstruction_velocity_gradient.clear();
  _reconstructed_pressure_gradient.clear();
  _coupling_pressure_gradient.clear();
  _coupling_pressure_gradient_initialized = false;
  transition(ReconstructionEvent::Reset);
}

void
FVReconstructedPressureGradient::saveLaggedVelocityGradient(RhieChowMassFlux & rc)
{
  checkFlowSystem(rc);
  transition(ReconstructionEvent::SaveLaggedGradient);
  const auto dimension = rc.dimension();

  mooseAssert(_velocity_gradient_fields.size() == dimension,
              "A velocity gradient field must be registered for every momentum component.");

  for (const auto component : make_range(dimension))
    rc.momentumSystem(component).updateFVGradient(*_velocity_gradient_fields[component]);

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
FVReconstructedPressureGradient::faceReconstructionError(const ElemInfo & elem_info,
                                                         const FaceInfo & fi,
                                                         const char * const problem) const
{
  mooseError("FVReconstructedPressureGradient '",
             name(),
             "' ",
             problem,
             " while reconstructing cell ID ",
             elem_info.elem()->id(),
             " from face ID ",
             fi.id(),
             ".");
}

void
FVReconstructedPressureGradient::assembleFaceProjection(const RhieChowMassFlux & rc,
                                                        const ElemInfo & elem_info,
                                                        const FaceInfo * const fi,
                                                        const Point & surface_vector,
                                                        const bool elem_has_info,
                                                        DenseMatrix<Real> & matrix,
                                                        DenseVector<Real> & projection_rhs,
                                                        std::vector<dof_id_type> & face_ids) const
{
  mooseAssert(fi, "FaceInfo must be available while reconstructing a cell.");

  face_ids.push_back(fi->id());

  const Real surface_area = surface_vector.norm();
  const auto face_normal = surface_vector / surface_area;
  // RhieChow stores the scalar flux relative to FaceInfo::normal(). Flip that orientation when the
  // current cell is on the opposite side so q_f is outward from this cell.
  const Point flux_normal =
      rc.hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->normal() : Point(-fi->normal());
  const Real face_flux = rc.getVolumetricFaceFlux(*fi);
  if (!std::isfinite(face_flux))
    faceReconstructionError(elem_info, *fi, "found a non-finite corrected face flux");
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
    for (const auto direction : make_range(rc.dimension()))
      if (!std::isfinite(velocity_gradient(direction)))
        faceReconstructionError(elem_info, *fi, "found a non-finite lagged velocity gradient");

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
    const Real reconstructed_velocity,
    const std::vector<dof_id_type> & face_ids) const
{
  const Elem & elem = *elem_info.elem();
  if (!std::isfinite(reconstructed_velocity))
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' computed a non-finite reconstructed velocity component ",
               component,
               " for cell ID ",
               elem.id(),
               " using face IDs ",
               Moose::stringify(face_ids),
               ".");

  const auto momentum_dof = elem_info.dofIndices()[rc.momentumSystem(component).number()][0];
  const Real HbyA = (*rc.HbyAComponents()[component])(momentum_dof);
  const Real Ainv = (*rc.AinvComponents()[component])(momentum_dof);
  if (!std::isfinite(HbyA) || !std::isfinite(Ainv) || Ainv == 0.0)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' found invalid momentum-coupling data for component ",
               component,
               " of cell ID ",
               elem.id(),
               " using face IDs ",
               Moose::stringify(face_ids),
               ": H/A = ",
               HbyA,
               " and 1/A = ",
               Ainv,
               ".");

  // Invert the same diagonal momentum relation used by Rhie-Chow,
  //   u_P = -(H/A)_P - A_P^{-1} (grad p)_P,
  // so the reconstructed pressure gradient produces the projected, flux-consistent u_P.
  const Real reconstructed_gradient = (-reconstructed_velocity - HbyA) / Ainv;
  if (!std::isfinite(reconstructed_gradient))
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' computed a non-finite pressure-gradient component ",
               component,
               " for cell ID ",
               elem.id(),
               " using face IDs ",
               Moose::stringify(face_ids),
               ".");

  return reconstructed_gradient;
}

void
FVReconstructedPressureGradient::computeCandidateFromCorrectedFlux(const RhieChowMassFlux & rc)
{
  checkFlowSystem(rc);
  transition(ReconstructionEvent::ReconstructCandidate);

  const auto dimension = rc.dimension();
  const auto & base_pressure_gradient = rc.basePressureGradientComponents();
  const auto face_flux_iteration = rc.faceMassFluxGeneration();

  if (face_flux_iteration == _last_reconstructed_face_flux_iteration)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' cannot reuse corrected face-flux iteration ",
               face_flux_iteration,
               " for reconstruction. The current pressure corrector must produce a new face "
               "flux before reconstruction.");

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
    std::vector<dof_id_type> face_ids;
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
          rc, *elem_info, fi, surface_vector, elem_has_info, matrix, projection_rhs, face_ids);
    };

    Moose::FV::loopOverElemFaceInfo(
        elem, mesh, act, mesh.getCoordSystem(elem.subdomain_id()), rz_radial_coord);

    const auto reconstructed_quantity = solveFaceProjection(matrix, projection_rhs);

    const auto pressure_dof =
        elem_info->dofIndices()[rc.pressureSystem().number()][rc.pressureVariableNumber()];
    for (const auto component : make_range(dimension))
    {
      const auto reconstructed_gradient = reconstructPressureGradient(
          rc, *elem_info, component, reconstructed_quantity(component), face_ids);
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
  if (_reconstruction_state != ReconstructionState::CandidateReady)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' has no reconstructed pressure-gradient candidate ready for the conservative "
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
  transition(ReconstructionEvent::PublishCandidate);
  updateCouplingPressureGradient(rc, base_gradient, _reconstructed_pressure_gradient);
}

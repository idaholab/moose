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
#include "libmesh/libmesh_exceptions.h"

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
FVReconstructedPressureGradient::linkFlowSystem(
    RhieChowMassFlux & rc, const LinearFVGradientReader & pressure_gradient) const
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
    if (!_base_gradient_method)
      _base_gradient_method = &resolveBaseGradientMethod(system);
    _base_gradient_method->computeGradient(system, gradient, variable_numbers);
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

  if (!_base_gradient_method)
    _base_gradient_method = &resolveBaseGradientMethod(system);

  bool coupling_gradient_layout_matches = _coupling_pressure_gradient_initialized &&
                                          _coupling_pressure_gradient.size() == gradient.size();
  if (coupling_gradient_layout_matches)
    for (const auto component : index_range(gradient))
      if (_coupling_pressure_gradient[component]->size() != gradient[component]->size() ||
          _coupling_pressure_gradient[component]->local_size() != gradient[component]->local_size())
      {
        coupling_gradient_layout_matches = false;
        break;
      }

  if (!coupling_gradient_layout_matches)
  {
    _base_gradient_method->computeGradient(system, gradient, variable_numbers);
    return;
  }

  for (const auto component : index_range(gradient))
    if (gradient[component]->type() == GHOSTED)
      _coupling_pressure_gradient[component]->localize(*gradient[component],
                                                       system.dofMap().get_send_list());
    else
      *gradient[component] = *_coupling_pressure_gradient[component];
}

void
FVReconstructedPressureGradient::copyGradient(const GradientView & source,
                                              GradientContainer & destination) const
{
  destination.resize(source.size());
  for (const auto component : index_range(source))
    if (!destination[component] || source[component]->size() != destination[component]->size() ||
        source[component]->local_size() != destination[component]->local_size() ||
        source[component]->first_local_index() != destination[component]->first_local_index() ||
        source[component]->last_local_index() != destination[component]->last_local_index())
      destination[component] = source[component]->clone();
    else
      *destination[component] = *source[component];
}

void
FVReconstructedPressureGradient::resetAttemptState() const
{
  _lagged_velocity_gradient_available = false;
  _lagged_velocity_gradient_generation = 0;
  _reconstructed_candidate_generation = 0;
  _reconstructed_candidate_face_flux_generation = 0;
  _published_candidate_generation = 0;
}

void
FVReconstructedPressureGradient::resetForTimeStep(const RhieChowMassFlux & rc) const
{
  checkFlowSystem(rc);
  copyGradient(rc.pressureGradientField().components(Moose::oldState()),
               _coupling_pressure_gradient);
  _coupling_pressure_gradient_initialized = true;
  resetAttemptState();
}

void
FVReconstructedPressureGradient::meshChanged()
{
  _lagged_reconstruction_velocity_gradient.clear();
  _reconstructed_pressure_gradient.clear();
  _coupling_pressure_gradient.clear();
  _coupling_pressure_gradient_initialized = false;
  resetAttemptState();
}

void
FVReconstructedPressureGradient::saveLaggedVelocityGradient(
    RhieChowMassFlux & rc) const
{
  checkFlowSystem(rc);
  const auto dimension = rc.dimension();

  mooseAssert(_velocity_gradient_fields.size() == dimension,
              "A velocity gradient field must be registered for every momentum component.");

  for (const auto component : make_range(dimension))
    rc.momentumSystem(component).updateFVGradient(*_velocity_gradient_fields[component]);

  if (_lagged_reconstruction_velocity_gradient.empty())
  {
    _lagged_reconstruction_velocity_gradient.resize(dimension);
    for (const auto component : make_range(dimension))
    {
      _lagged_reconstruction_velocity_gradient[component].resize(dimension);
      for (const auto direction : make_range(dimension))
        _lagged_reconstruction_velocity_gradient[component][direction] =
            _velocity_gradient_fields[component]->components()[direction]->zero_clone();
    }
  }

  for (const auto component : make_range(dimension))
    for (const auto direction : make_range(dimension))
      *_lagged_reconstruction_velocity_gradient[component][direction] =
          *_velocity_gradient_fields[component]->components()[direction];

  _lagged_velocity_gradient_available = true;
  ++_lagged_velocity_gradient_generation;
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
FVReconstructedPressureGradient::computeCandidateFromCorrectedFlux(
    const RhieChowMassFlux & rc) const
{
  checkFlowSystem(rc);
  if (!rc.momentumPredictorGeneration())
    mooseError("A momentum predictor must be prepared before updating a reconstructed "
               "pressure-gradient candidate.");
  if (!_lagged_velocity_gradient_available)
    mooseError("A lagged velocity-gradient snapshot must exist before updating a reconstructed "
               "pressure-gradient candidate.");

  const auto next_candidate_generation = _reconstructed_candidate_generation + 1;
  if (_lagged_velocity_gradient_generation != next_candidate_generation)
    mooseError("Reconstructed pressure-gradient candidate generation ",
               next_candidate_generation,
               " requires lagged velocity-gradient generation ",
               next_candidate_generation,
               ", but generation ",
               _lagged_velocity_gradient_generation,
               " is available.");

  const auto dimension = rc.dimension();
  const auto & base_pressure_gradient = rc.basePressureGradientComponents();
  const auto face_flux_generation = rc.faceMassFluxGeneration();
  const auto expected_face_flux_generation = _reconstructed_candidate_face_flux_generation + 1;

  if (_reconstructed_pressure_gradient.empty())
    for (const auto component : make_range(dimension))
      _reconstructed_pressure_gradient.push_back(base_pressure_gradient[component]->zero_clone());

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

    auto act = [&](const Elem &,
                   const Elem * const,
                   const FaceInfo * const fi,
                   const Point & surface_vector,
                   const Real,
                   const bool elem_has_info)
    {
      if (!fi)
        mooseError("FVReconstructedPressureGradient '",
                   name(),
                   "' encountered missing face information while reconstructing cell ID ",
                   elem.id(),
                   ".");

      face_ids.push_back(fi->id());

      if (face_flux_generation != expected_face_flux_generation)
        mooseError("FVReconstructedPressureGradient '",
                   name(),
                   "' cannot reconstruct cell ID ",
                   elem.id(),
                   " from face ID ",
                   fi->id(),
                   ": expected corrected face-flux generation ",
                   expected_face_flux_generation,
                   " but found generation ",
                   face_flux_generation,
                   ". The face flux is stale or was not produced by the current pressure "
                   "corrector.");

      const Real surface_area = surface_vector.norm();
      if (!std::isfinite(surface_area) || surface_area <= 0.0)
        mooseError("FVReconstructedPressureGradient '",
                   name(),
                   "' found invalid surface area ",
                   surface_area,
                   " while reconstructing cell ID ",
                   elem.id(),
                   " from face ID ",
                   fi->id(),
                   ".");

      for (const auto component : make_range(dimension))
        if (!std::isfinite(surface_vector(component)))
          mooseError("FVReconstructedPressureGradient '",
                     name(),
                     "' found a non-finite surface-vector component while reconstructing cell ID ",
                     elem.id(),
                     " from face ID ",
                     fi->id(),
                     ".");

      const auto face_normal = surface_vector / surface_area;
      const Point flux_normal =
          rc.hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->normal() : Point(-fi->normal());
      const Real face_flux = rc.getVolumetricFaceFlux(*fi);
      const Real normal_alignment = flux_normal * face_normal;
      if (!std::isfinite(face_flux) || !std::isfinite(normal_alignment))
        mooseError("FVReconstructedPressureGradient '",
                   name(),
                   "' found non-finite face-flux projection data while reconstructing cell ID ",
                   elem.id(),
                   " from face ID ",
                   fi->id(),
                   ".");
      Real face_normal_reconstructed_quantity = face_flux * normal_alignment;

      const Point d_pf = fi->faceCentroid() - elem_info->centroid();
      Real gradient_flux_correction = 0.0;
      for (const auto component : make_range(dimension))
      {
        const auto velocity_gradient =
            reconstructionVelocityGradient(rc, *elem_info, *fi, elem_has_info, component);
        for (const auto direction : make_range(dimension))
          if (!std::isfinite(velocity_gradient(direction)) || !std::isfinite(d_pf(direction)))
            mooseError("FVReconstructedPressureGradient '",
                       name(),
                       "' found non-finite velocity-gradient or face-displacement data while "
                       "reconstructing cell ID ",
                       elem.id(),
                       " from face ID ",
                       fi->id(),
                       ".");

        gradient_flux_correction +=
            (velocity_gradient * d_pf) * surface_vector(component);
      }

      if (!std::isfinite(gradient_flux_correction))
        mooseError("FVReconstructedPressureGradient '",
                   name(),
                   "' computed a non-finite velocity-gradient correction while reconstructing "
                   "cell ID ",
                   elem.id(),
                   " from face ID ",
                   fi->id(),
                   ".");

      face_normal_reconstructed_quantity -= gradient_flux_correction / surface_area;
      if (!std::isfinite(face_normal_reconstructed_quantity))
        mooseError("FVReconstructedPressureGradient '",
                   name(),
                   "' computed a non-finite corrected face quantity while reconstructing cell ID ",
                   elem.id(),
                   " from face ID ",
                   fi->id(),
                   ".");

      for (const auto i : make_range(dimension))
      {
        projection_rhs(i) += face_normal_reconstructed_quantity * surface_vector(i);
        for (const auto j : make_range(dimension))
        {
          matrix(i, j) += surface_vector(i) * surface_vector(j) / surface_area;
          if (!std::isfinite(matrix(i, j)))
            mooseError("FVReconstructedPressureGradient '",
                       name(),
                       "' assembled a non-finite projection matrix while reconstructing cell ID ",
                       elem.id(),
                       " from face ID ",
                       fi->id(),
                       ".");
        }

        if (!std::isfinite(projection_rhs(i)))
          mooseError("FVReconstructedPressureGradient '",
                     name(),
                     "' assembled a non-finite projection right-hand side while reconstructing "
                     "cell ID ",
                     elem.id(),
                     " from face ID ",
                     fi->id(),
                     ".");
      }
    };

    Moose::FV::loopOverElemFaceInfo(
        elem, mesh, act, mesh.getCoordSystem(elem.subdomain_id()), rz_radial_coord);

    if (face_ids.empty())
      mooseError("FVReconstructedPressureGradient '",
                 name(),
                 "' found no corrected faces while reconstructing cell ID ",
                 elem.id(),
                 ".");

    DenseVector<Real> reconstructed_quantity(dimension);
    if (dimension == 1)
    {
      const Real denominator = matrix(0, 0);
      if (!std::isfinite(denominator) || denominator <= 0.0)
        mooseError("FVReconstructedPressureGradient '",
                   name(),
                   "' could not factor the face-projection matrix for cell ID ",
                   elem.id(),
                   " using face IDs ",
                   Moose::stringify(face_ids),
                   ": the one-dimensional matrix entry is ",
                   denominator,
                   ".");
      reconstructed_quantity(0) = projection_rhs(0) / denominator;
    }
    else
    {
      DenseMatrix<Real> solve_matrix(matrix);
      try
      {
        solve_matrix.cholesky_solve(projection_rhs, reconstructed_quantity);
      }
      catch (const libMesh::LogicError & error)
      {
        mooseError("FVReconstructedPressureGradient '",
                   name(),
                   "' could not factor the face-projection matrix for cell ID ",
                   elem.id(),
                   " using face IDs ",
                   Moose::stringify(face_ids),
                   ". The corrected face geometry does not span the reconstruction space. "
                   "libMesh reported: ",
                   error.what());
      }
    }

    for (const auto component : make_range(dimension))
    {
      if (!std::isfinite(reconstructed_quantity(component)))
        mooseError("FVReconstructedPressureGradient '",
                   name(),
                   "' computed a non-finite reconstructed velocity component ",
                   component,
                   " for cell ID ",
                   elem.id(),
                   " using face IDs ",
                   Moose::stringify(face_ids),
                   ".");

      const auto momentum_dof =
          elem_info->dofIndices()[rc.momentumSystem(component).number()][0];
      const auto pressure_dof = elem_info->dofIndices()[rc.pressureSystem().number()]
                                                      [rc.pressureVariableNumber()];

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

      const Real reconstructed_gradient = (-reconstructed_quantity(component) - HbyA) / Ainv;
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

      _reconstructed_pressure_gradient[component]->set(pressure_dof, reconstructed_gradient);
    }
  }

  for (auto & pressure_gradient : _reconstructed_pressure_gradient)
    pressure_gradient->close();

  _reconstructed_candidate_generation = next_candidate_generation;
  _reconstructed_candidate_face_flux_generation = face_flux_generation;
}

const FVReconstructedPressureGradient::GradientContainer &
FVReconstructedPressureGradient::reconstructedCandidate(const RhieChowMassFlux & rc) const
{
  checkFlowSystem(rc);
  mooseAssert(_reconstructed_candidate_generation == _published_candidate_generation + 1,
              "The reconstructed pressure-gradient candidate must be formed exactly once before "
              "it is used for the conservative cell-velocity correction.");
  return _reconstructed_pressure_gradient;
}

void
FVReconstructedPressureGradient::updateCouplingPressureGradient(
    const GradientView & base_gradient, const GradientContainer & reconstructed_candidate) const
{
  const auto num_components = base_gradient.size();
  if (num_components == 0 || reconstructed_candidate.size() != num_components)
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' requires nonempty base and reconstructed gradients with equal component "
               "counts.");

  for (const auto component : index_range(base_gradient))
    if (base_gradient[component]->size() != reconstructed_candidate[component]->size() ||
        base_gradient[component]->local_size() !=
            reconstructed_candidate[component]->local_size())
      mooseError("FVReconstructedPressureGradient '",
                 name(),
                 "' requires base and reconstructed gradient components with equal layouts.");

  bool storage_matches = _coupling_pressure_gradient.size() == num_components;
  if (storage_matches)
    for (const auto component : index_range(_coupling_pressure_gradient))
      if (_coupling_pressure_gradient[component]->size() != base_gradient[component]->size() ||
          _coupling_pressure_gradient[component]->local_size() !=
              base_gradient[component]->local_size())
      {
        storage_matches = false;
        break;
      }

  if (!storage_matches)
    copyGradient(base_gradient, _coupling_pressure_gradient);

  if (!storage_matches || !_coupling_pressure_gradient_initialized)
  {
    for (const auto component : index_range(_coupling_pressure_gradient))
    {
      *_coupling_pressure_gradient[component] = *base_gradient[component];
      _coupling_pressure_gradient[component]->close();
    }
    _coupling_pressure_gradient_initialized = true;
  }

  for (const auto component : index_range(_coupling_pressure_gradient))
  {
    _coupling_pressure_gradient[component]->scale(1.0 - _gradient_relaxation);
    _coupling_pressure_gradient[component]->add(_gradient_relaxation,
                                                *reconstructed_candidate[component]);
    _coupling_pressure_gradient[component]->close();
  }

}

void
FVReconstructedPressureGradient::publishCouplingPressureGradient(
    const RhieChowMassFlux & rc, const GradientView & base_gradient) const
{
  checkFlowSystem(rc);
  mooseAssert(_reconstructed_candidate_generation == _published_candidate_generation + 1,
              "Each reconstructed pressure-gradient candidate must be published exactly once.");
  updateCouplingPressureGradient(base_gradient, _reconstructed_pressure_gradient);
  _published_candidate_generation = _reconstructed_candidate_generation;
}

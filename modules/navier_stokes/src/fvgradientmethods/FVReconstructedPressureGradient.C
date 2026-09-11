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
  params.addParam<MooseEnum>(
      "reconstructed_pressure_gradient_boundary_cells",
      MooseEnum("reconstructed base_gradient", "reconstructed"),
      "Which pressure gradient is retained on cells touching a boundary face.");
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
    _reconstructed_pressure_gradient_boundary_cells(
        getParam<MooseEnum>("reconstructed_pressure_gradient_boundary_cells")),
    _gradient_relaxation(getParam<Real>("gradient_relaxation"))
{
}

void
FVReconstructedPressureGradient::bindFlowSystem(
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
    mooseError("FVReconstructedPressureGradient '",
               name(),
               "' must be bound to one RhieChow flow system before it computes gradients.");

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
    *gradient[component] = *_coupling_pressure_gradient[component];
}

void
FVReconstructedPressureGradient::resetForTimeStep(const RhieChowMassFlux & rc) const
{
  checkFlowSystem(rc);
  _coupling_pressure_gradient_initialized = false;
  _lagged_velocity_gradient_available = false;
  _lagged_velocity_gradient_generation = 0;
  _reconstructed_candidate_generation = 0;
  _reconstructed_candidate_face_flux_generation = 0;
  _published_candidate_generation = 0;
}

void
FVReconstructedPressureGradient::meshChanged()
{
  _boundary_cell_cache_built = false;
  _boundary_cell_ids.clear();
  _lagged_reconstruction_velocity_gradient.clear();
  _reconstructed_pressure_gradient.clear();
  _coupling_pressure_gradient.clear();
  if (_rhie_chow)
    resetForTimeStep(*_rhie_chow);
}

void
FVReconstructedPressureGradient::buildBoundaryCellCache(const RhieChowMassFlux & rc) const
{
  _boundary_cell_ids.clear();
  for (const auto & fi : rc.pressureSystem().feProblem().mesh().faceInfo())
    if (!fi->boundaryIDs().empty())
    {
      if (fi->elemPtr() && rc.hasBlocks(fi->elemPtr()->subdomain_id()))
        _boundary_cell_ids.insert(fi->elemPtr()->id());
      if (fi->neighborPtr() && rc.hasBlocks(fi->neighborPtr()->subdomain_id()))
        _boundary_cell_ids.insert(fi->neighborPtr()->id());
    }

  _boundary_cell_cache_built = true;
}

void
FVReconstructedPressureGradient::captureLaggedVelocityGradient(
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
  mooseAssert(rc.momentumPredictorGeneration(),
              "A momentum predictor must be prepared before updating a reconstructed "
              "pressure-gradient candidate.");
  mooseAssert(_lagged_velocity_gradient_available,
              "A lagged velocity-gradient snapshot must exist before updating a reconstructed "
              "pressure-gradient candidate.");

  const auto next_candidate_generation = _reconstructed_candidate_generation + 1;
  mooseAssert(_lagged_velocity_gradient_generation >= next_candidate_generation,
              "The lagged velocity-gradient snapshot must be captured before forming a "
              "reconstructed pressure-gradient candidate.");
  mooseAssert(_lagged_velocity_gradient_generation == next_candidate_generation,
              "Each reconstructed pressure-gradient candidate must consume exactly one lagged "
              "velocity-gradient snapshot.");
  _reconstructed_candidate_generation = next_candidate_generation;

  mooseAssert(rc.faceMassFluxGeneration() != _reconstructed_candidate_face_flux_generation,
              "Each reconstructed pressure-gradient candidate must consume a face mass flux "
              "computed by the current pressure corrector.");
  _reconstructed_candidate_face_flux_generation = rc.faceMassFluxGeneration();

  if (!_boundary_cell_cache_built)
    buildBoundaryCellCache(rc);

  const auto dimension = rc.dimension();
  const bool use_base_gradient_on_boundary =
      _reconstructed_pressure_gradient_boundary_cells == "base_gradient";
  const auto & base_pressure_gradient = rc.basePressureGradientComponents();

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

    const bool boundary_cell = _boundary_cell_ids.count(elem_info->elem()->id());
    DenseMatrix<Real> matrix(dimension, dimension);
    DenseVector<Real> projection_rhs(dimension);
    matrix.zero();
    projection_rhs.zero();

    const Elem & elem = *elem_info->elem();
    auto act = [&](const Elem &,
                   const Elem * const,
                   const FaceInfo * const fi,
                   const Point & surface_vector,
                   const Real,
                   const bool elem_has_info)
    {
      const Real surface_area = surface_vector.norm();
      if (surface_area == 0.0)
        return;

      const auto face_normal = surface_vector / surface_area;
      const Point flux_normal =
          rc.hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->normal() : Point(-fi->normal());
      Real face_normal_reconstructed_quantity =
          rc.getVolumetricFaceFlux(*fi) * (flux_normal * face_normal);

      const Point d_pf = fi->faceCentroid() - elem_info->centroid();
      Real gradient_flux_correction = 0.0;
      for (const auto component : make_range(dimension))
        gradient_flux_correction +=
            (reconstructionVelocityGradient(rc, *elem_info, *fi, elem_has_info, component) * d_pf) *
            surface_vector(component);

      face_normal_reconstructed_quantity -= gradient_flux_correction / surface_area;

      for (const auto i : make_range(dimension))
      {
        projection_rhs(i) += face_normal_reconstructed_quantity * surface_vector(i);
        for (const auto j : make_range(dimension))
          matrix(i, j) += surface_vector(i) * surface_vector(j) / surface_area;
      }
    };

    Moose::FV::loopOverElemFaceInfo(
        elem, mesh, act, mesh.getCoordSystem(elem.subdomain_id()), rz_radial_coord);

    DenseVector<Real> reconstructed_quantity(dimension);
    if (dimension == 1)
    {
      const Real denominator = matrix(0, 0);
      reconstructed_quantity(0) =
          denominator != 0.0 ? projection_rhs(0) / denominator : 0.0;
    }
    else
    {
      DenseMatrix<Real> solve_matrix(matrix);
      solve_matrix.cholesky_solve(projection_rhs, reconstructed_quantity);
    }

    for (const auto component : make_range(dimension))
    {
      const auto momentum_dof =
          elem_info->dofIndices()[rc.momentumSystem(component).number()][0];
      const auto pressure_dof = elem_info->dofIndices()[rc.pressureSystem().number()]
                                                      [rc.pressureVariableNumber()];

      const Real HbyA = (*rc.HbyAComponents()[component])(momentum_dof);
      const Real Ainv = (*rc.AinvComponents()[component])(momentum_dof);
      const Real base_gradient = (*base_pressure_gradient[component])(pressure_dof);
      const Real reconstructed_gradient =
          Ainv != 0.0 ? (-reconstructed_quantity(component) - HbyA) / Ainv : base_gradient;
      const Real stored_gradient =
          use_base_gradient_on_boundary && boundary_cell ? base_gradient : reconstructed_gradient;

      _reconstructed_pressure_gradient[component]->set(pressure_dof, stored_gradient);
    }
  }

  for (auto & pressure_gradient : _reconstructed_pressure_gradient)
    pressure_gradient->close();

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
    const GradientContainer & base_gradient,
    const GradientContainer & reconstructed_candidate) const
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
  {
    _coupling_pressure_gradient.clear();
    for (const auto component : index_range(base_gradient))
      _coupling_pressure_gradient.push_back(base_gradient[component]->clone());
  }

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
    const RhieChowMassFlux & rc, const GradientContainer & base_gradient) const
{
  checkFlowSystem(rc);
  mooseAssert(_reconstructed_candidate_generation == _published_candidate_generation + 1,
              "Each reconstructed pressure-gradient candidate must be published exactly once.");
  updateCouplingPressureGradient(base_gradient, _reconstructed_pressure_gradient);
  _published_candidate_generation = _reconstructed_candidate_generation;
}

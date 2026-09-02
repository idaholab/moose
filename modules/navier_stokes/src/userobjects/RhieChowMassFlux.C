//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "RhieChowMassFlux.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "NS.h"
#include "VectorCompositeFunctor.h"
#include "PIMPLE.h"
#include "SIMPLE.h"
#include "PetscVectorReader.h"
#include "LinearSystem.h"
#include "LinearFVGradientInterface.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearFVAdvectionDiffusionFunctorDirichletBC.h"
#include "LinearFVPressureCorrectionDiffusion.h"
#include "LinearFVMomentumPressure.h"
#include "LinearFVPressureFluxBC.h"
#include "FVReconstructedPressureGradient.h"
#include "FVUtils.h"
#include "LinearFVAnisotropicDiffusion.h"
#include "MooseUtils.h"

// libMesh includes
#include "libmesh/mesh_base.h"
#include "libmesh/elem.h"
#include "libmesh/elem_range.h"
#include "libmesh/petsc_matrix.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", RhieChowMassFlux);

InputParameters
RhieChowMassFlux::validParams()
{
  auto params = RhieChowFaceFluxProvider::validParams();
  params += NonADFunctorInterface::validParams();

  params.addClassDescription("Computes H/A and 1/A together with face mass fluxes for segregated "
                             "momentum-pressure equations using linear systems.");

  params.addRequiredParam<VariableName>(NS::pressure, "The pressure variable.");
  params.addRequiredParam<VariableName>("u", "The x-component of velocity");
  params.addParam<VariableName>("v", "The y-component of velocity");
  params.addParam<VariableName>("w", "The z-component of velocity");
  params.addRequiredParam<std::string>(
      "p_diffusion_kernel",
      "The LinearFVPressureCorrectionDiffusion kernel acting on the pressure.");

  params.addRequiredParam<MooseFunctorName>(NS::density, "Density functor");

  // We disable the execution of this, should only provide functions
  // for the SIMPLE executioner
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_NONE);
  exec_enum = {EXEC_NONE};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  // Pressure projection
  params.addParam<MooseEnum>("pressure_projection_method",
                             MooseEnum("standard consistent", "standard"),
                             "The method to use in the pressure projection for Ainv - "
                             "standard (SIMPLE) or consistent (SIMPLEC)");
  params.addParam<MooseEnum>(
      "pressure_diffusion_interpolation",
      MooseEnum("average harmonic", "average"),
      "The face interpolation method for Ainv in the pressure correction diffusion term.");
  params.addParam<bool>(
      "enforce_coupling_pressure_gradient_identity",
      false,
      "Whether to enforce that the pressure gradient removed while constructing H/A matches "
      "the registered momentum-coupling pressure gradient field. Intended primarily for "
      "diagnostic tests.");
  params.addRangeCheckedParam<Real>(
      "coupling_pressure_gradient_identity_tolerance",
      1e-12,
      "coupling_pressure_gradient_identity_tolerance >= 0.0",
      "Tolerance for the discrete identity between the momentum pressure-source gradient and "
      "the gradient removed while constructing H/A when "
      "'enforce_coupling_pressure_gradient_identity' is true.");
  return params;
}

RhieChowMassFlux::RhieChowMassFlux(const InputParameters & params)
  : RhieChowFaceFluxProvider(params),
    NonADFunctorInterface(this),
    _moose_mesh(UserObject::_subproblem.mesh()),
    _mesh(_moose_mesh.getMesh()),
    _dim(blocksMaxDimension()),
    _p(dynamic_cast<MooseLinearVariableFVReal *>(
        &UserObject::_subproblem.getVariable(0, getParam<VariableName>(NS::pressure)))),
    _vel(_dim, nullptr),
    _HbyA_flux(_moose_mesh, blockIDs(), "HbyA_flux"),
    _Ainv(_moose_mesh, blockIDs(), "Ainv"),
    _face_mass_flux(
        declareRestartableData<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>>(
            "face_flux", _moose_mesh, blockIDs(), "face_values")),
    _rho(getFunctor<Real>(NS::density)),
    _pressure_system(nullptr),
    _pressure_gradient_field(nullptr),
    _base_pressure_gradient_field(nullptr),
    _global_pressure_system_number(0),
    _pressure_projection_method(getParam<MooseEnum>("pressure_projection_method")),
    _pressure_diffusion_interp_method(getParam<MooseEnum>("pressure_diffusion_interpolation") ==
                                              "harmonic"
                                          ? Moose::FV::InterpMethod::HarmonicAverage
                                          : Moose::FV::InterpMethod::Average),
    _enforce_coupling_pressure_gradient_identity(
        getParam<bool>("enforce_coupling_pressure_gradient_identity")),
    _coupling_pressure_gradient_identity_tolerance(
        getParam<Real>("coupling_pressure_gradient_identity_tolerance"))
{
  if (!_p)
    paramError(NS::pressure, "the pressure must be a MooseLinearVariableFVReal.");
  checkBlocks(*_p);

  std::vector<std::string> vel_names = {"u", "v", "w"};
  for (const auto i : index_range(_vel))
  {
    _vel[i] = dynamic_cast<MooseLinearVariableFVReal *>(
        &UserObject::_subproblem.getVariable(0, getParam<VariableName>(vel_names[i])));

    if (!_vel[i])
      paramError(vel_names[i], "the velocity must be a MOOSELinearVariableFVReal.");
    checkBlocks(*_vel[i]);
  }

  // Register the elemental/face functors which will be queried in the pressure equation
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("Ainv", _Ainv, tid);
    UserObject::_subproblem.addFunctor("HbyA", _HbyA_flux, tid);
  }

  if (!dynamic_cast<SIMPLE *>(getMooseApp().getExecutioner()) &&
      !dynamic_cast<PIMPLE *>(getMooseApp().getExecutioner()))
    mooseError(this->name(),
               " should only be used with a linear segregated thermal-hydraulics solver!");
}

void
RhieChowMassFlux::linkMomentumPressureSystems(
    const std::vector<LinearSystem *> & momentum_systems,
    LinearSystem & pressure_system,
    const std::vector<unsigned int> & momentum_system_numbers)
{
  _momentum_systems = momentum_systems;
  _momentum_system_numbers = momentum_system_numbers;
  _pressure_system = &pressure_system;
  _global_pressure_system_number = _pressure_system->number();

  auto * const pressure_var =
      dynamic_cast<MooseLinearVariableFVReal *>(&_pressure_system->getVariable(0, _p->number()));
  if (!pressure_var)
    mooseError("The pressure variable in system '",
               _pressure_system->name(),
               "' must be a MooseLinearVariableFVReal.");

  // For each momentum system, find the LinearFVMomentumPressure kernels applicable to this RC
  // object, i.e. those whose blocks overlap the blocks this RC object handles. Kernels entirely
  // outside our blocks are irrelevant and ignored; multiple applicable kernels per system are
  // allowed as long as they partition our blocks without overlapping (checked below).
  std::vector<std::vector<const LinearFVMomentumPressure *>> applicable_kernels(
      momentum_systems.size());
  for (const auto system_index : index_range(momentum_systems))
  {
    std::vector<LinearFVElementalKernel *> kernels;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribThread>(_tid)
        .template condition<AttribSystem>("LinearFVElementalKernel")
        .template condition<AttribSysNum>(momentum_system_numbers[system_index])
        .queryInto(kernels);

    for (const auto * const kernel : kernels)
    {
      const auto * const candidate = dynamic_cast<const LinearFVMomentumPressure *>(kernel);
      if (candidate && MooseUtils::setsIntersect(candidate->blockIDs(), blockIDs()))
        applicable_kernels[system_index].push_back(candidate);
    }
  }

  for (const auto system_index : index_range(momentum_systems))
  {
    const auto & kernels = applicable_kernels[system_index];
    const auto & system_name = momentum_systems[system_index]->name();

    // Coverage: the applicable kernels must together cover every block this RC object handles.
    std::set<SubdomainID> covered_blocks;
    for (const auto * const kernel : kernels)
      covered_blocks.insert(kernel->blockIDs().begin(), kernel->blockIDs().end());

    std::set<SubdomainID> missing_blocks;
    std::set_difference(blockIDs().begin(),
                        blockIDs().end(),
                        covered_blocks.begin(),
                        covered_blocks.end(),
                        std::inserter(missing_blocks, missing_blocks.end()));

    if (!missing_blocks.empty())
      mooseError("RhieChowMassFlux '",
                 name(),
                 "' found no applicable LinearFVMomentumPressure kernel for momentum system '",
                 system_name,
                 "' on block(s) ",
                 Moose::stringify(missing_blocks),
                 ".");

    // Component and pairwise block-overlap checks among the applicable kernels for this system.
    for (const auto i : index_range(kernels))
    {
      if (kernels[i]->momentumComponent() != system_index)
        mooseError("LinearFVMomentumPressure kernel '",
                   kernels[i]->name(),
                   "' declares momentum component ",
                   kernels[i]->momentumComponent(),
                   " but is linked to momentum system '",
                   system_name,
                   "' (component ",
                   system_index,
                   ") through RhieChowMassFlux '",
                   name(),
                   "'.");

      for (const auto j : make_range(i + 1, kernels.size()))
      {
        std::set<SubdomainID> overlap;
        std::set_intersection(kernels[i]->blockIDs().begin(),
                              kernels[i]->blockIDs().end(),
                              kernels[j]->blockIDs().begin(),
                              kernels[j]->blockIDs().end(),
                              std::inserter(overlap, overlap.end()));
        if (!overlap.empty())
          mooseError("Momentum system '",
                     system_name,
                     "' has more than one LinearFVMomentumPressure kernel contributing a "
                     "pressure source on the same block(s) for RhieChowMassFlux '",
                     name(),
                     "': '",
                     kernels[i]->name(),
                     "' and '",
                     kernels[j]->name(),
                     "' overlap on block(s) ",
                     Moose::stringify(overlap),
                     ".");
      }
    }
  }

  // Flatten every system's applicable kernels into one list spanning the whole RC object. Even
  // kernels on disjoint blocks/components must share one coupling-gradient field, because H/A
  // has a single field source for this RC object.
  std::vector<const LinearFVMomentumPressure *> momentum_pressure_kernels;
  for (auto & kernels : applicable_kernels)
    momentum_pressure_kernels.insert(
        momentum_pressure_kernels.end(), kernels.begin(), kernels.end());

  const auto * const coupling_kernel = momentum_pressure_kernels.front();
  const auto & coupling_reader = coupling_kernel->pressureGradientField();

  if (dynamic_cast<const FVReconstructedPressureGradient *>(&coupling_reader.method()) &&
      coupling_kernel->pressureVariableNumber() != _p->number())
    mooseError("FVReconstructedPressureGradient can only be used for the pressure variable "
               "registered on RhieChowMassFlux '",
               name(),
               "'.");

  for (const auto * const kernel : momentum_pressure_kernels)
  {
    const auto & reader = kernel->pressureGradientField();

    if (const auto * const reconstructed_method =
            dynamic_cast<const FVReconstructedPressureGradient *>(&reader.method()))
      reconstructed_method->bindFlowSystem(*this, reader);

    if (&reader.system() != &coupling_reader.system() ||
        reader.systemNumber() != coupling_reader.systemNumber())
      mooseError("RhieChowMassFlux '",
                 name(),
                 "': momentum pressure kernels '",
                 coupling_kernel->name(),
                 "' (block(s) ",
                 Moose::stringify(coupling_kernel->blockIDs()),
                 ") and '",
                 kernel->name(),
                 "' (block(s) ",
                 Moose::stringify(kernel->blockIDs()),
                 ") must use pressure gradients from the same system.");

    if (reader.variableNumber() != coupling_reader.variableNumber())
      mooseError("RhieChowMassFlux '",
                 name(),
                 "': momentum pressure kernels '",
                 coupling_kernel->name(),
                 "' (pressure variable number ",
                 coupling_reader.variableNumber(),
                 ") and '",
                 kernel->name(),
                 "' (pressure variable number ",
                 reader.variableNumber(),
                 ") must use the same pressure variable.");

    if (&reader.method() != &coupling_reader.method())
      mooseError("RhieChowMassFlux '",
                 name(),
                 "': momentum pressure kernels '",
                 coupling_kernel->name(),
                 "' and '",
                 kernel->name(),
                 "' must use the same pressure gradient method.");

    if (&reader.components() != &coupling_reader.components())
      mooseError("RhieChowMassFlux '",
                 name(),
                 "': momentum pressure kernels '",
                 coupling_kernel->name(),
                 "' and '",
                 kernel->name(),
                 "' must share the same registered pressure gradient field.");
  }

  _pressure_gradient_field = &coupling_reader;

  if (usingReconstructedPressureGradientMethod())
  {
    const auto & base_reader = pressure_var->requestCellGradients(
        reconstructedGradientMethod().baseGradientMethodName());
    _base_pressure_gradient_field = &base_reader;

    mooseAssert(_base_pressure_gradient_field != _pressure_gradient_field,
                "Reconstructed and base pressure gradient readers must be distinct when "
                "FVReconstructedPressureGradient is active.");
  }
  else
    _base_pressure_gradient_field = _pressure_gradient_field;

  _global_momentum_system_numbers.clear();
  _momentum_implicit_systems.clear();
  for (auto & system : _momentum_systems)
  {
    _global_momentum_system_numbers.push_back(system->number());
    _momentum_implicit_systems.push_back(dynamic_cast<LinearImplicitSystem *>(&system->system()));
  }

  if (usingReconstructedPressureGradientMethod())
  {
    checkReconstructedPressureGradientCompatibility();
    if (_p_diffusion_kernel && _p_diffusion_kernel->useNonorthogonalCorrection() &&
        _pressure_projection_method != "consistent")
      paramError("pressure_projection_method",
                 "FVReconstructedPressureGradient with nonorthogonal pressure diffusion requires "
                 "the consistent pressure projection.");
  }

  setupMeshInformation();

  std::set<BoundaryID> velocity_boundary_ids;
  for (const auto dim_i : make_range(_dim))
    for (const auto & [boundary_id, _] : _vel[dim_i]->getBoundaryConditionMap())
      velocity_boundary_ids.insert(boundary_id);

  const auto is_dirichlet =
      [](const MooseLinearVariableFVReal & variable, const BoundaryID boundary_id)
  {
    return dynamic_cast<const LinearFVAdvectionDiffusionFunctorDirichletBC *>(
               variable.getBoundaryCondition(boundary_id)) != nullptr;
  };

  // The legacy boundary HbyA reconstruction uses the x-velocity BC as a proxy for every velocity
  // component, so all components must have the same Dirichlet classification on those boundaries.
  for (const auto boundary_id : velocity_boundary_ids)
  {
    const auto * const pressure_bc = _p->getBoundaryCondition(boundary_id);
    if (dynamic_cast<const LinearFVPressureFluxBC *>(pressure_bc))
      continue;

    const bool velocity_is_dirichlet = is_dirichlet(*_vel[0], boundary_id);
    for (const auto dim_i : make_range(_dim))
      if (is_dirichlet(*_vel[dim_i], boundary_id) != velocity_is_dirichlet)
        mooseError("All velocity components must either have Dirichlet boundary conditions or "
                   "non-Dirichlet boundary conditions on boundary '",
                   _moose_mesh.getBoundaryName(boundary_id),
                   "' when the pressure boundary condition is not a LinearFVPressureFluxBC.");
  }
}

void
RhieChowMassFlux::meshChanged()
{
  _HbyA_flux.clear();
  _Ainv.clear();
  _face_mass_flux.clear();
  _face_mass_flux_generation = 0;
  setupMeshInformation();

}

void
RhieChowMassFlux::timestepSetup()
{
  // Nothing to reset when momentum hasn't been linked (e.g. should_solve_momentum = false, used
  // by the restart-freeze pattern) or when a reconstructed pressure gradient isn't in use.
  if (!_pressure_gradient_field || !usingReconstructedPressureGradientMethod())
    return;

  // The reconstructed gradient's feedback/candidate/generation state is solver-iteration state,
  // not an independent physical solution: discard it so this time step's momentum predictor
  // starts from the base pressure gradient regardless of whether the preceding step ran
  // continuously, was recovered from a checkpoint, or is a retry after a rejected step (see the
  // class-level comment on the declaration for why this hook fires exactly once per attempt).
  reconstructedGradientMethod().resetForTimeStep(*this);

  // This counter is otherwise self-refreshing every SIMPLE/PISO iteration, but resetting it here
  // avoids any accidental cross-time-step coupling and keeps the reconstructed gradient's paired
  // safety-assertion counter meaningful within a single time step.
  _face_mass_flux_generation = 0;
}

void
RhieChowMassFlux::prepareMomentumPredictor()
{
  if (!usingReconstructedPressureGradientMethod())
    return;

  reconstructedGradientMethod().captureLaggedVelocityGradient(*this);
}

void
RhieChowMassFlux::preparePressureRelaxation()
{
  if (!usingReconstructedPressureGradientMethod())
    return;

  // The current pressure solution and corrected face flux are both unrelaxed here. Form the
  // conservative candidate before pressure relaxation replaces that solution.
  _pressure_system->updateFVGradient(basePressureGradientField());
  reconstructedGradientMethod().computeCandidateFromCorrectedFlux(*this);
  updateCellVelocity(reconstructedGradientMethod().reconstructedCandidate(*this));
}

void
RhieChowMassFlux::finalizePressureCorrector()
{
  // Refresh the ordinary base gradient from the relaxed pressure solution.
  _pressure_system->updateFVGradient(basePressureGradientField());

  if (usingReconstructedPressureGradientMethod())
  {
    reconstructedGradientMethod().publishRelaxedFeedback(*this, basePressureGradientComponents());
    _pressure_system->updateFVGradient(pressureGradientField());
  }
  else
    updateCellVelocity(pressureGradientComponents());
}

void
RhieChowMassFlux::initPressureGradient()
{
  _pressure_system->updateFVGradient(basePressureGradientField());
  _pressure_system->updateFVGradient(pressureGradientField());
}

void
RhieChowMassFlux::initialSetup()
{
  // We fetch the pressure diffusion kernel to ensure that the face flux correction
  // is consistent with the pressure discretization in the Poisson equation.
  std::vector<LinearFVFluxKernel *> flux_kernel;
  auto base_query = _fe_problem.theWarehouse()
                        .query()
                        .template condition<AttribThread>(_tid)
                        .template condition<AttribSysNum>(_p->sys().number())
                        .template condition<AttribSystem>("LinearFVFluxKernel")
                        .template condition<AttribName>(getParam<std::string>("p_diffusion_kernel"))
                        .queryInto(flux_kernel);
  if (flux_kernel.size() != 1)
    paramError(
        "p_diffusion_kernel",
        "The kernel with the given name could not be found or multiple instances were identified.");
  _p_diffusion_kernel = dynamic_cast<LinearFVPressureCorrectionDiffusion *>(flux_kernel[0]);
  if (!_p_diffusion_kernel)
    paramError("p_diffusion_kernel",
               "The provided diffusion kernel must be of type "
               "LinearFVPressureCorrectionDiffusion.");
}

void
RhieChowMassFlux::setupMeshInformation()
{
  // We cache the cell volumes into a petsc vector for corrections here so we can use
  // the optimized petsc operations for the normalization
  _cell_volumes = _pressure_system->currentSolution()->zero_clone();
  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
    // We have to check this because the variable might not be defined on the given
    // block
    if (hasBlocks(elem_info->subdomain_id()))
    {
      const auto elem_dof = elem_info->dofIndices()[_global_pressure_system_number][0];
      _cell_volumes->set(elem_dof, elem_info->volume() * elem_info->coordFactor());
    }

  _cell_volumes->close();

  _flow_face_info.clear();
  for (auto & fi : _fe_problem.mesh().faceInfo())
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
      _flow_face_info.push_back(fi);
}

void
RhieChowMassFlux::updateCellVelocity(
    const std::vector<std::unique_ptr<NumericVector<Number>>> & pressure_gradient)
{
  // u_C = -(H/A)_C - (1/A)_C * grad(p)_C.
  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    auto working_vector = _Ainv_raw[system_i]->clone();
    working_vector->pointwise_mult(*working_vector, *pressure_gradient[system_i]);
    working_vector->add(*_HbyA_raw[system_i]);
    working_vector->scale(-1.0);
    (*_momentum_implicit_systems[system_i]->solution) = *working_vector;

#ifndef NDEBUG
    {
      // Proof that the assignment above satisfies the momentum-coupling identity
      // u_C + (H/A)_C + Ainv_C * grad(p)_C = 0. The solution was just assigned directly from
      // this formula, so the residual is pure floating-point round-off from the
      // clone/scale/add sequence, not solver error; 1e-10 is a loose absolute bound with
      // headroom over that round-off.
      auto defect = _Ainv_raw[system_i]->clone();
      defect->pointwise_mult(*defect, *pressure_gradient[system_i]);
      defect->add(*_HbyA_raw[system_i]);
      defect->add(*_momentum_implicit_systems[system_i]->solution);
      mooseAssert(MooseUtils::absoluteFuzzyEqual(defect->linfty_norm(), 0.0, 1e-10),
                  "RhieChowMassFlux: cell-velocity update violates the momentum-coupling "
                  "identity u + H/A + Ainv*grad(p) = 0.");
    }
#endif

    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }
}

void
RhieChowMassFlux::initialize()
{
  for (const auto & pair : _HbyA_flux)
    _HbyA_flux[pair.first] = 0;

  for (const auto & pair : _Ainv)
    _Ainv[pair.first] = 0;
}

void
RhieChowMassFlux::initFaceMassFlux()
{
  using namespace Moose::FV;

  const auto time_arg = Moose::currentState();

  // We loop through the faces and compute the resulting face fluxes from the
  // initial conditions for velocity
  for (auto & fi : _flow_face_info)
  {
    RealVectorValue density_times_velocity;

    // On internal face we do a regular interpolation with geometric weights
    if (_vel[0]->isInternalFace(*fi))
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();

      Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
      Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);

      for (const auto dim_i : index_range(_vel))
        interpolate(InterpMethod::Average,
                    density_times_velocity(dim_i),
                    _vel[dim_i]->getElemValue(elem_info, time_arg) * elem_rho,
                    _vel[dim_i]->getElemValue(neighbor_info, time_arg) * neighbor_rho,
                    *fi,
                    true);
    }
    // On the boundary, we just take the boundary values
    else
    {
      const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
      const Elem * const boundary_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();

      // We need this multiplier in case the face is an internal face and
      const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};

      const Real face_rho = _rho(boundary_face, time_arg);
      for (const auto dim_i : index_range(_vel))
        density_times_velocity(dim_i) = boundary_normal_multiplier * face_rho *
                                        raw_value((*_vel[dim_i])(boundary_face, time_arg));
    }

    _face_mass_flux[fi->id()] = density_times_velocity * fi->normal();
  }
}

Real
RhieChowMassFlux::getMassFlux(const FaceInfo & fi) const
{
  return _face_mass_flux.evaluate(&fi);
}

Real
RhieChowMassFlux::getVolumetricFaceFlux(const FaceInfo & fi) const
{
  const Moose::FaceArg face_arg{&fi,
                                /*limiter_type=*/Moose::FV::LimiterType::CentralDifference,
                                /*elem_is_upwind=*/true,
                                /*correct_skewness=*/false,
                                &fi.elem(),
                                /*state_limiter*/ nullptr};
  const Real face_rho = _rho(face_arg, Moose::currentState());
  return libmesh_map_find(_face_mass_flux, fi.id()) / face_rho;
}

const LinearFVGradientReader &
RhieChowMassFlux::pressureGradientField() const
{
  if (!_pressure_gradient_field)
    mooseError(
        "The pressure gradient field has not been registered for RhieChowMassFlux '", name(), "'.");

  return *_pressure_gradient_field;
}

const LinearFVGradientReader &
RhieChowMassFlux::basePressureGradientField() const
{
  if (!_base_pressure_gradient_field)
    mooseError("The base pressure gradient field has not been registered for RhieChowMassFlux '",
               name(),
               "'.");

  return *_base_pressure_gradient_field;
}

const std::vector<std::unique_ptr<NumericVector<Number>>> &
RhieChowMassFlux::HbyAComponents() const
{
  mooseAssert(!_HbyA_raw.empty(), "HbyA data is not ready.");

  return _HbyA_raw;
}

const std::vector<std::unique_ptr<NumericVector<Number>>> &
RhieChowMassFlux::AinvComponents() const
{
  mooseAssert(!_Ainv_raw.empty(), "Ainv data is not ready.");

  return _Ainv_raw;
}

const std::vector<std::unique_ptr<NumericVector<Number>>> &
RhieChowMassFlux::basePressureGradientComponents() const
{
  return basePressureGradientField().components();
}

unsigned int
RhieChowMassFlux::pressureVariableNumber() const
{
  return _p->number();
}

Real
RhieChowMassFlux::getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                                        const FaceInfo & fi,
                                        const Moose::StateArg & time,
                                        const THREAD_ID /*tid*/,
                                        bool libmesh_dbg_var(subtract_mesh_velocity)) const
{
  mooseAssert(!subtract_mesh_velocity, "RhieChowMassFlux does not support moving meshes yet!");

  if (m != Moose::FV::InterpMethod::RhieChow)
    mooseError("Interpolation methods other than Rhie-Chow are not supported!");
  if (time.state != Moose::currentState().state)
    mooseError("Older interpolation times are not supported!");

  return getVolumetricFaceFlux(fi);
}

void
RhieChowMassFlux::computeFaceMassFlux()
{
  using namespace Moose::FV;

  const auto time_arg = Moose::currentState();

  // Petsc vector reader to make the repeated reading from the vector faster
  PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);

  // We loop through the faces and compute the face fluxes using the pressure gradient
  // and the momentum matrix/right hand side
  for (auto & fi : _flow_face_info)
  {
    // Making sure the kernel knows which face we are on
    _p_diffusion_kernel->setupFaceData(fi);

    // We are setting this to 1.0 because we don't want to multiply the kernel contributions
    // with the surface area yet. The surface area will be factored in in the advection kernels.
    _p_diffusion_kernel->setCurrentFaceArea(1.0);

    Real p_grad_flux = 0.0;
    if (_p->isInternalFace(*fi))
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();

      // Fetching the dof indices for the pressure variable
      const auto elem_dof = elem_info.dofIndices()[_global_pressure_system_number][0];
      const auto neighbor_dof = neighbor_info.dofIndices()[_global_pressure_system_number][0];

      // Fetching the values of the pressure for the element and the neighbor
      const auto p_elem_value = p_reader(elem_dof);
      const auto p_neighbor_value = p_reader(neighbor_dof);

      // Compute the elem matrix contributions for the face
      const auto elem_matrix_contribution = _p_diffusion_kernel->computeElemMatrixContribution();
      const auto neighbor_matrix_contribution =
          _p_diffusion_kernel->computeNeighborMatrixContribution();
      const auto elem_rhs_contribution =
          _p_diffusion_kernel->computeElemRightHandSideContribution();

      // Compute the face flux from the matrix and right hand side contributions
      p_grad_flux = (p_neighbor_value * neighbor_matrix_contribution +
                     p_elem_value * elem_matrix_contribution) -
                    elem_rhs_contribution;
    }
    else if (auto * bc_pointer = _p->getBoundaryCondition(*fi->boundaryIDs().begin()))
    {
      mooseAssert(fi->boundaryIDs().size() == 1, "We should only have one boundary on every face.");

      bc_pointer->setupFaceData(
          fi, fi->faceType(std::make_pair(_p->number(), _global_pressure_system_number)));

      const ElemInfo & elem_info =
          hasBlocks(fi->elemPtr()->subdomain_id()) ? *fi->elemInfo() : *fi->neighborInfo();
      const auto p_elem_value = _p->getElemValue(elem_info, time_arg);
      const auto matrix_contribution =
          _p_diffusion_kernel->computeBoundaryMatrixContribution(*bc_pointer);
      const auto rhs_contribution =
          _p_diffusion_kernel->computeBoundaryRHSContribution(*bc_pointer);

      // On the boundary, only the element side has a contribution
      p_grad_flux = (p_elem_value * matrix_contribution - rhs_contribution);
    }
    // Compute the new face flux
    _face_mass_flux[fi->id()] = -_HbyA_flux[fi->id()] + p_grad_flux;
  }

  ++_face_mass_flux_generation;
}

void
RhieChowMassFlux::initCouplingField()
{
  // We loop through the faces and populate the coupling fields (face H/A and 1/H)
  // with 0s for now. Pressure corrector solves will always come after the
  // momentum source so we expect these fields to change before the actual solve.
  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    _Ainv[fi->id()];
    _HbyA_flux[fi->id()];
  }
}

void
RhieChowMassFlux::populateCouplingFunctors(
    const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya,
    const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_Ainv)
{
  // We have the raw H/A and 1/A vectors in a petsc format. This function
  // will create face functors from them
  using namespace Moose::FV;
  const auto time_arg = Moose::currentState();

  // Create the petsc vector readers for faster repeated access
  std::vector<PetscVectorReader> hbya_reader;
  for (const auto dim_i : index_range(raw_hbya))
    hbya_reader.emplace_back(*raw_hbya[dim_i]);

  std::vector<PetscVectorReader> ainv_reader;
  for (const auto dim_i : index_range(raw_Ainv))
    ainv_reader.emplace_back(*raw_Ainv[dim_i]);

  // We loop through the faces and populate the coupling fields (face H/A and 1/H)
  for (auto & fi : _flow_face_info)
  {
    Real face_rho = 0;
    RealVectorValue face_hbya;

    // We do the lookup in advance
    auto & Ainv = _Ainv[fi->id()];

    // If it is internal, we just interpolate (using geometric weights) to the face
    if (_vel[0]->isInternalFace(*fi))
    {
      // Get the dof indices for the element and the neighbor
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[0]][0];
      const auto neighbor_dof = neighbor_info.dofIndices()[_global_momentum_system_numbers[0]][0];

      // Get the density values for the element and neighbor. We need this multiplication to make
      // the coupling fields mass fluxes.
      const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
      const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);

      // Now we do the interpolation to the face
      interpolate(Moose::FV::InterpMethod::Average, face_rho, elem_rho, neighbor_rho, *fi, true);
      for (const auto dim_i : index_range(raw_hbya))
      {
        interpolate(Moose::FV::InterpMethod::Average,
                    face_hbya(dim_i),
                    hbya_reader[dim_i](elem_dof),
                    hbya_reader[dim_i](neighbor_dof),
                    *fi,
                    true);
        interpolate(_pressure_diffusion_interp_method,
                    Ainv(dim_i),
                    elem_rho * ainv_reader[dim_i](elem_dof),
                    neighbor_rho * ainv_reader[dim_i](neighbor_dof),
                    *fi,
                    true);
      }
    }
    else
    {
      const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());

      // We need this multiplier in case the face is an internal face and
      const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;

      const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[0]][0];

      mooseAssert(fi->boundaryIDs().size() == 1, "We should only have one boundary on every face.");
      const auto * pressure_bc = _p->getBoundaryCondition(*fi->boundaryIDs().begin());

      // For the legacy Dirichlet-velocity plus extrapolated-pressure path, we still need a
      // special boundary HbyA reconstruction to keep the face flux consistent. When the pressure
      // BC itself is a LinearFVPressureFluxBC, that object already enforces the prescribed
      // boundary mass flux, so we use the standard one-term boundary expansion instead.
      if (_vel[0]->isDirichletBoundaryFace(*fi) &&
          !dynamic_cast<const LinearFVPressureFluxBC *>(pressure_bc))
      {
        const Moose::FaceArg boundary_face{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};
        face_rho = _rho(boundary_face, Moose::currentState());

        for (const auto dim_i : make_range(_dim))
        {

          face_hbya(dim_i) =
              -MetaPhysicL::raw_value((*_vel[dim_i])(boundary_face, Moose::currentState()));
          face_hbya(dim_i) *= boundary_normal_multiplier;
        }
      }
      // Otherwise we just do a one-term expansion (so we just use the element value)
      else
      {
        const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[0]][0];

        face_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
        for (const auto dim_i : make_range(_dim))
          face_hbya(dim_i) = boundary_normal_multiplier * hbya_reader[dim_i](elem_dof);
      }

      // We just do a one-term expansion for 1/A no matter what
      const Real elem_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
      for (const auto dim_i : index_range(raw_Ainv))
        Ainv(dim_i) = elem_rho * ainv_reader[dim_i](elem_dof);
    }
    // Lastly, we populate the face flux resulted by H/A
    _HbyA_flux[fi->id()] = face_hbya * fi->normal() * face_rho;
  }
}

void
RhieChowMassFlux::computeHbyA(const bool with_updated_pressure, bool verbose)
{
  if (verbose)
  {
    _console << "************************************" << std::endl;
    _console << "Computing HbyA" << std::endl;
    _console << "************************************" << std::endl;
  }
  mooseAssert(_momentum_implicit_systems.size() && _momentum_implicit_systems[0],
              "The momentum system shall be linked before calling this function!");

  // Select (and, if requested, snapshot) the momentum-coupling pressure gradient
  // used by all LinearFVMomentumPressure kernels so that the discrete vector
  // removed from the momentum RHS here matches the one assembled into it.
  auto & coupling_pressure_gradient = selectPressureGradient(with_updated_pressure);

  if (_enforce_coupling_pressure_gradient_identity && with_updated_pressure)
    checkCouplingPressureGradientIdentity();

  _HbyA_raw.clear();
  _Ainv_raw.clear();

  for (auto system_i : index_range(_momentum_systems))
  {
    LinearImplicitSystem * momentum_system = _momentum_implicit_systems[system_i];

    NumericVector<Number> & rhs = *(momentum_system->rhs);
    NumericVector<Number> & current_local_solution = *(momentum_system->current_local_solution);
    NumericVector<Number> & solution = *(momentum_system->solution);
    PetscMatrix<Number> * mmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system->matrix);
    mooseAssert(mmat,
                "The matrices used in the segregated INSFVRhieChow objects need to be convertable "
                "to PetscMatrix!");

    if (verbose)
    {
      _console << "Matrix in rc object" << std::endl;
      mmat->print();
    }

    // First, we extract the diagonal and we will hold on to it for a little while
    _Ainv_raw.push_back(current_local_solution.zero_clone());
    NumericVector<Number> & Ainv = *(_Ainv_raw.back());

    mmat->get_diagonal(Ainv);

    if (verbose)
    {
      _console << "Velocity solution in H(u)" << std::endl;
      solution.print();
    }

    // Time to create H(u) = M_{offdiag} * u - b_{nonpressure}
    _HbyA_raw.push_back(current_local_solution.zero_clone());
    NumericVector<Number> & HbyA = *(_HbyA_raw.back());

    // We start with the matrix product part, we will do
    // M*u - A*u for 2 reasons:
    // 1, We assume A*u petsc operation is faster than setting the matrix diagonal to 0
    // 2, In PISO loops we need to reuse the matrix so we can't just set the diagonals to 0

    // We create a working vector to ease some of the operations, we initialize its values
    // with the current solution values to have something for the A*u term
    auto working_vector = momentum_system->current_local_solution->zero_clone();
    PetscVector<Number> * working_vector_petsc =
        dynamic_cast<PetscVector<Number> *>(working_vector.get());
    mooseAssert(working_vector_petsc,
                "The vectors used in the RhieChowMassFlux objects need to be convertable "
                "to PetscVectors!");

    mmat->vector_mult(HbyA, solution);
    working_vector_petsc->pointwise_mult(Ainv, solution);
    HbyA.add(-1.0, *working_vector_petsc);

    if (verbose)
    {
      _console << " H(u)" << std::endl;
      HbyA.print();
    }

    // We continue by adding the momentum right hand side contributions
    HbyA.add(-1.0, rhs);

    // Unfortunately, the pressure forces are included in the momentum RHS
    // so we have to correct them back using the same coupling gradient that
    // assembled the momentum pressure source.
    working_vector_petsc->pointwise_mult(*coupling_pressure_gradient[system_i], *_cell_volumes);
    HbyA.add(-1.0, *working_vector_petsc);

    if (verbose)
    {
      _console << "total RHS" << std::endl;
      rhs.print();
      _console << "pressure RHS" << std::endl;
      coupling_pressure_gradient[system_i]->print();
      _console << " H(u)-rhs-relaxation_source" << std::endl;
      HbyA.print();
    }

    // It is time to create element-wise 1/A-s based on the the diagonal of the momentum matrix
    *working_vector_petsc = 1.0;
    Ainv.pointwise_divide(*working_vector_petsc, Ainv);

    // Create 1/A*(H(u)-RHS)
    HbyA.pointwise_mult(HbyA, Ainv);

    if (verbose)
    {
      _console << " (H(u)-rhs)/A" << std::endl;
      HbyA.print();
    }

    if (_pressure_projection_method == "consistent")
    {

      // Consistent Corrections to SIMPLE
      // 1. Ainv_old = 1/a_p <- Ainv = 1/(a_p + \sum_n a_n)
      // 2. H(u) <- H(u*) + H(u') = H(u*) - (Ainv - Ainv_old) * grad(p) * Vc

      if (verbose)
        _console << "Performing SIMPLEC projection." << std::endl;

      // Lambda function to calculate the sum of diagonal and neighbor coefficients
      auto get_row_sum = [mmat](NumericVector<Number> & sum_vector)
      {
        // Ensure the sum_vector is zeroed out
        sum_vector.zero();

        // Local row size
        const auto local_size = mmat->local_m();

        for (const auto row_i : make_range(local_size))
        {
          // Get all non-zero components of the row of the matrix
          const auto global_index = mmat->row_start() + row_i;
          std::vector<numeric_index_type> indices;
          std::vector<Real> values;
          mmat->get_row(global_index, indices, values);

          // Sum row elements (no absolute values)
          const Real row_sum = std::accumulate(values.cbegin(), values.cend(), 0.0);

          // Add the sum of diagonal and elements to the sum_vector
          sum_vector.add(global_index, row_sum);
        }
        sum_vector.close();
      };

      // Create a temporary vector to store the sum of diagonal and neighbor coefficients
      auto row_sum = current_local_solution.zero_clone();
      get_row_sum(*row_sum);

      // Create vector with new inverse projection matrix
      auto Ainv_full = current_local_solution.zero_clone();
      *working_vector_petsc = 1.0;
      Ainv_full->pointwise_divide(*working_vector_petsc, *row_sum);
      const auto Ainv_full_old = Ainv_full->clone();

      // Correct HbyA
      Ainv_full->add(-1.0, Ainv);
      working_vector_petsc->pointwise_mult(*Ainv_full, *coupling_pressure_gradient[system_i]);
      working_vector_petsc->pointwise_mult(*working_vector_petsc, *_cell_volumes);
      HbyA.add(-1.0, *working_vector_petsc);

      // Correct Ainv
      Ainv = *Ainv_full_old;
    }

    Ainv.pointwise_mult(Ainv, *_cell_volumes);

    if (verbose)
    {
      _console << " 1/A" << std::endl;
      Ainv.print();
    }
  }

  // We fill the 1/A and H/A functors
  populateCouplingFunctors(_HbyA_raw, _Ainv_raw);

  if (verbose)
  {
    _console << "************************************" << std::endl;
    _console << "DONE Computing HbyA " << std::endl;
    _console << "************************************" << std::endl;
  }
}

std::vector<std::unique_ptr<NumericVector<Number>>> &
RhieChowMassFlux::selectPressureGradient(const bool updated_pressure)
{
  // The pressure-gradient field removed while constructing H/A must be the same
  // coupling field used by the LinearFVMomentumPressure kernels. In reconstructed
  // mode, this is the reconstructed gradient, not the base gradient.
  const auto & coupling_pressure_gradient_components = pressureGradientComponents();

  // The snapshot is only (re-)taken when updated_pressure is true, i.e. on the first PISO
  // corrector of a sequence. Later correctors in the same sequence reuse it unchanged, even
  // though the reconstructed feedback gradient keeps changing corrector to corrector: the
  // momentum matrix/RHS built for this sequence already has that first-corrector gradient baked
  // into its pressure source, so computeHbyA() must keep subtracting the same gradient it used
  // originally until the next momentum predictor rebuilds the matrix from scratch.
  if (updated_pressure)
  {
    _grad_p_current.clear();
    for (const auto & component : coupling_pressure_gradient_components)
      _grad_p_current.push_back(component->clone());
  }

  if (_grad_p_current.empty())
    for (const auto & component : coupling_pressure_gradient_components)
      _grad_p_current.push_back(component->clone());

  return _grad_p_current;
}

const std::vector<std::unique_ptr<NumericVector<Number>>> &
RhieChowMassFlux::pressureGradientComponents() const
{
  return pressureGradientField().components();
}

bool
RhieChowMassFlux::usingReconstructedPressureGradientMethod() const
{
  return dynamic_cast<const FVReconstructedPressureGradient *>(&pressureGradientField().method());
}

const FVReconstructedPressureGradient &
RhieChowMassFlux::reconstructedGradientMethod() const
{
  mooseAssert(usingReconstructedPressureGradientMethod(),
              "reconstructedGradientMethod() should only be called when reconstructed "
              "pressure gradients are active.");

  return dynamic_cast<const FVReconstructedPressureGradient &>(pressureGradientField().method());
}

void
RhieChowMassFlux::checkCouplingPressureGradientIdentity() const
{
  const auto & coupling_components = pressureGradientComponents();

  if (coupling_components.empty())
    mooseError("RhieChowMassFlux '",
               name(),
               "' has no registered pressure gradient components when enforcing the coupling "
               "pressure-gradient identity.");

  if (_grad_p_current.size() != coupling_components.size())
    mooseError(
        "RhieChowMassFlux '",
        name(),
        "' requires the pressure-gradient snapshot used to construct H/A to have the same "
        "number of components as the registered momentum-coupling pressure gradient field when "
        "enforcing the coupling pressure-gradient identity.");

  Real max_difference = 0.0;

  for (const auto component : index_range(coupling_components))
  {
    const auto & coupling_vec = *coupling_components[component];
    const auto & snapshot_vec = *_grad_p_current[component];

    if (coupling_vec.size() != snapshot_vec.size() ||
        coupling_vec.local_size() != snapshot_vec.local_size())
      mooseError("RhieChowMassFlux '",
                 name(),
                 "' requires the pressure-gradient snapshot used to construct H/A to have the "
                 "same layout as the registered momentum-coupling gradient field when enforcing "
                 "the coupling pressure-gradient identity.");

    auto diff = coupling_vec.clone();
    diff->add(-1.0, snapshot_vec);
    const Real component_max = diff->linfty_norm();
    if (component_max > max_difference)
      max_difference = component_max;
  }

  if (max_difference > _coupling_pressure_gradient_identity_tolerance)
    mooseError("RhieChowMassFlux '",
               name(),
               "' violates the coupling pressure-gradient identity: the maximum difference "
               "between the registered momentum-coupling pressure gradient and the gradient "
               "removed while constructing H/A is ",
               max_difference,
               ", which exceeds the tolerance ",
               _coupling_pressure_gradient_identity_tolerance,
               ".");
}

void
RhieChowMassFlux::checkReconstructedPressureGradientCompatibility() const
{
  mooseAssert(_pressure_system,
              "The pressure system should be linked before compatibility checks.");

  if (_pressure_system->nVariables() != 1)
    mooseError(
        "FVReconstructedPressureGradient assumes the pressure and momentum systems each have "
        "exactly one variable so their DOF indices can be used in vector operations. Pressure "
        "system '",
        _pressure_system->name(),
        "' has variables: ",
        Moose::stringify(_pressure_system->getVariableNames()));

  for (const auto system_i : index_range(_momentum_systems))
  {
    const auto * const momentum_system = _momentum_systems[system_i];
    mooseAssert(momentum_system, "Momentum system pointer should not be null.");

    if (momentum_system->nVariables() != 1)
      mooseError("FVReconstructedPressureGradient assumes the pressure and momentum systems each "
                 "have exactly one variable so their DOF indices can be used in vector operations. "
                 "Momentum system '",
                 momentum_system->name(),
                 "' has variables: ",
                 Moose::stringify(momentum_system->getVariableNames()));
  }
}

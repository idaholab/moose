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
#include "MathFVUtils.h"
#include "FVUtils.h"
#include "PetscVectorReader.h"
#include "LinearSystem.h"
#include "LinearFVGradientManager.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearFVAdvectionDiffusionFunctorDirichletBC.h"
#include "LinearFVPressureCorrectionDiffusion.h"
#include "LinearFVMomentumPressure.h"
#include "LinearFVPressureFluxBC.h"
#include "FVReconstructedPressureGradient.h"
#include "MooseUtils.h"
#include "LinearFVFluxKernel.h"
#include "NSFVUtils.h"

// libMesh includes
#include "libmesh/mesh_base.h"
#include "libmesh/elem.h"
#include "libmesh/elem_range.h"
#include "libmesh/petsc_matrix.h"

#include <cmath>

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
    _pressure_equation_flux(_moose_mesh, blockIDs(), "pressure_equation_flux"),
    _rho(getFunctor<Real>(NS::density)),
    _pressure_system(nullptr),
    _pressure_gradient_field(nullptr),
    _base_pressure_gradient_field(nullptr),
    _global_pressure_system_number(0),
    _pressure_projection_method(getParam<MooseEnum>("pressure_projection_method")),
    _pressure_diffusion_interp_method(getParam<MooseEnum>("pressure_diffusion_interpolation") ==
                                              "harmonic"
                                          ? Moose::FV::InterpMethod::HarmonicAverage
                                          : Moose::FV::InterpMethod::Average)
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
  for (const auto tid : make_range(UserObject::_subproblem.numThreads()))
  {
    UserObject::_subproblem.addFunctor("Ainv", _Ainv, tid);
    UserObject::_subproblem.addFunctor("HbyA", _HbyA_flux, tid);
  }

  // Any executioner built on the segregated linear-assembly solve carries its parameters
  if (!getMooseApp().getExecutioner()->parameters().have_parameter<std::vector<SolverSystemName>>(
          "momentum_systems"))
    mooseError(this->name(),
               " should only be used with a linear segregated thermal-hydraulics solver!");
}

RhieChowMassFlux::~RhieChowMassFlux() = default;

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

  // A split predictor operator carries no explicit pressure term, so there is no
  // LinearFVMomentumPressure kernel to read the coupling gradient off. The provider adds that
  // force itself, through addMomentumPredictorExplicitForcing, and so states the gradient here
  // instead of inferring it.
  const bool split_predictor = splitMomentumPredictorOperator();

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
      if (!split_predictor && candidate &&
          MooseUtils::setsIntersect(candidate->blockIDs(), blockIDs()))
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

    if (!split_predictor && !missing_blocks.empty())
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
        for (const auto block : kernels[i]->blockIDs())
          if (hasBlocks(block) && kernels[j]->blockIDs().count(block))
            overlap.insert(block);
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

  const auto * const coupling_kernel =
      split_predictor ? nullptr : momentum_pressure_kernels.front();
  const auto & coupling_reader = split_predictor ? pressure_var->requestCellGradients()
                                                 : coupling_kernel->pressureGradientField();

  if (coupling_kernel &&
      dynamic_cast<const FVReconstructedPressureGradient *>(&coupling_reader.method()) &&
      coupling_kernel->pressureVariableNumber() != _p->number())
    mooseError("FVReconstructedPressureGradient can only be used for the pressure variable "
               "registered on RhieChowMassFlux '",
               name(),
               "'.");

  for (const auto * const kernel : momentum_pressure_kernels)
  {
    const auto & reader = kernel->pressureGradientField();

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
    reconstructedGradientMethod().linkFlowSystem(*this, coupling_reader);
    const auto & base_reader =
        pressure_var->requestCellGradients(reconstructedGradientMethod().baseGradientMethodName());
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
  _grad_p_current.clear();
  _pressure_equation_flux.clear();
  _pressure_equation_flux_valid = false;
  setupMeshInformation();
}

void
RhieChowMassFlux::timestepSetup()
{
  // Nothing to reset when momentum hasn't been linked (e.g. should_solve_momentum = false, used
  // by the restart-freeze pattern).
  if (!_pressure_gradient_field)
    return;

  _grad_p_current.clear();

  if (!usingReconstructedPressureGradientMethod())
    return;

  // Seed the next attempt from the most recently accepted coupling gradient. The linear FV
  // gradient manager restores that state after rejection and checkpoints it for restart/recovery.
  reconstructedGradientMethod().resetForTimeStep(*this);
}

void
RhieChowMassFlux::prepareMomentumPredictor()
{
  mooseAssert(_pressure_gradient_field,
              "The pressure gradient field must be linked before preparing momentum.");

  _grad_p_current.clear();
  for (const auto & component : pressureGradientComponents())
    _grad_p_current.push_back(component->clone());
}

void
RhieChowMassFlux::finalizeMomentumPredictor()
{
  if (usingReconstructedPressureGradientMethod())
    reconstructedGradientMethod().saveLaggedVelocityGradient(*this);
}

void
RhieChowMassFlux::preparePISOCorrector()
{
  if (!usingReconstructedPressureGradientMethod())
    return;

  mooseAssert(!_grad_p_current.empty(),
              "A momentum predictor must be prepared before another PISO corrector.");

  for (const auto component : make_range(dimension()))
    momentumSystem(component).updateFVGradient(velocityVariable(component).requestCellGradients());
  reconstructedGradientMethod().saveLaggedVelocityGradient(*this);
}

void
RhieChowMassFlux::preparePressureRelaxation()
{
  if (!usingReconstructedPressureGradientMethod())
    return;

  // The current pressure solution and corrected face flux are both unrelaxed here. Form the
  // conservative candidate before pressure relaxation replaces that solution.
  reconstructedGradientMethod().computeCandidateFromCorrectedFlux(*this);
  updateCellVelocity(reconstructedGradientMethod().reconstructedCandidate(*this));
}

void
RhieChowMassFlux::finalizePressureCorrector()
{
  if (usingReconstructedPressureGradientMethod())
  {
    // Refresh every ordinary pressure gradient while leaving the old coupling field untouched
    // until the current reconstructed candidate has been relaxed into it.
    _pressure_system->computeGradientsExcept(pressureGradientField());
    reconstructedGradientMethod().finalizeCouplingPressureGradient(
        *this, basePressureGradientComponents());
    _pressure_system->updateFVGradient(pressureGradientField());
  }
  else
  {
    _pressure_system->computeGradients();
    // Through the seam, not straight to the arithmetic: a provider that reconstructs the
    // corrected velocity differently has to be given the chance to.
    computeCellVelocity();
  }
}

void
RhieChowMassFlux::initPressureGradient()
{
  _pressure_system->computeGradients();
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
  _cell_fluid_volumes = _pressure_system->currentSolution()->zero_clone();
  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
    // We have to check this because the variable might not be defined on the given
    // block
    if (hasBlocks(elem_info->subdomain_id()))
    {
      const auto elem_dof = elem_info->dofIndices()[_global_pressure_system_number][0];
      _cell_fluid_volumes->set(elem_dof, elem_info->volume() * elem_info->coordFactor());
    }

  _cell_fluid_volumes->close();

  _flow_face_info.clear();
  for (auto & fi : _fe_problem.mesh().faceInfo())
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
      _flow_face_info.push_back(fi);
}

template <typename GradientComponent>
void
RhieChowMassFlux::updateCellVelocity(const std::vector<GradientComponent> & pressure_gradient)
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

  for (const auto & pair : _pressure_equation_flux)
    _pressure_equation_flux[pair.first] = 0;

  _pressure_equation_flux_valid = false;
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
      const Real boundary_normal_multiplier = boundaryNormalMultiplier(*fi);
      // makeCDFace names the flow side, the one hasFaceSide reports as covered by our blocks
      const Moose::FaceArg boundary_face = makeCDFace(*fi);

      const Real face_rho = _rho(boundary_face, time_arg);
      for (const auto dim_i : index_range(_vel))
        density_times_velocity(dim_i) =
            boundary_normal_multiplier * face_rho * boundaryVelocityValue(*fi, dim_i, time_arg);
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
  const std::string neighbor_cell_id =
      fi.neighborPtr() ? std::to_string(fi.neighborPtr()->id()) : "none";
  const auto face_flux = _face_mass_flux.find(fi.id());
  if (face_flux == _face_mass_flux.end())
    mooseError("RhieChowMassFlux '",
               name(),
               "' has no corrected mass flux for face ID ",
               fi.id(),
               " adjacent to cell ID ",
               fi.elem().id(),
               " and neighbor cell ID ",
               neighbor_cell_id,
               ".");

  if (!std::isfinite(face_flux->second))
    mooseError("RhieChowMassFlux '",
               name(),
               "' has non-finite corrected mass flux ",
               face_flux->second,
               " on face ID ",
               fi.id(),
               " adjacent to cell ID ",
               fi.elem().id(),
               " and neighbor cell ID ",
               neighbor_cell_id,
               ".");

  // On a face shared with a non-flow block, the FaceInfo element may be the non-flow cell, so
  // the density must be evaluated on the flow side.
  const Elem * face_side = fi.elemPtr();
  if (!_vel[0]->isInternalFace(fi) && !hasBlocks(fi.elemPtr()->subdomain_id()))
    face_side = fi.neighborPtr();

  const Moose::FaceArg face_arg{&fi,
                                /*limiter_type=*/Moose::FV::LimiterType::CentralDifference,
                                /*elem_is_upwind=*/true,
                                /*correct_skewness=*/false,
                                face_side,
                                /*state_limiter*/ nullptr};
  const Real face_rho = _rho(face_arg, Moose::currentState());
  if (!std::isfinite(face_rho) || face_rho <= 0.0)
    mooseError("RhieChowMassFlux '",
               name(),
               "' requires finite, positive density for corrected face flux reconstruction, but "
               "density is ",
               face_rho,
               " on face ID ",
               fi.id(),
               " adjacent to cell ID ",
               fi.elem().id(),
               " and neighbor cell ID ",
               neighbor_cell_id,
               ".");

  const Real volumetric_flux = face_flux->second / face_rho;
  if (!std::isfinite(volumetric_flux))
    mooseError("RhieChowMassFlux '",
               name(),
               "' computed non-finite volumetric flux ",
               volumetric_flux,
               " on face ID ",
               fi.id(),
               " adjacent to cell ID ",
               fi.elem().id(),
               " and neighbor cell ID ",
               neighbor_cell_id,
               ".");

  return volumetric_flux;
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

const std::vector<NumericVector<Number> *> &
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
RhieChowMassFlux::cachePressureEquationFlux()
{
  // The anisotropic diffusion kernel's nonorthogonal RHS contribution uses
  // cell pressure gradients. When we recache the solved pressure face flux
  // after a pressure solve / relaxation step, refresh those gradients first so
  // the reconstructed pressure-equation flux stays aligned with the current pressure field.
  const_cast<MooseLinearVariableFVReal *>(_p)->computeCellGradients();

  // Petsc vector reader to make the repeated reading from the vector faster
  PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);
  for (auto & fi : _flow_face_info)
    _pressure_equation_flux[fi->id()] = computeFacePressureGradientFlux(*fi, p_reader);

  _pressure_equation_flux_valid = true;
}

void
RhieChowMassFlux::computeFaceMassFlux()
{
  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  for (auto & fi : _flow_face_info)
    _face_mass_flux[fi->id()] = -_HbyA_flux[fi->id()] + _pressure_equation_flux[fi->id()];

  ++_face_mass_flux_generation;
}

Real
RhieChowMassFlux::computeFacePressureGradientFlux(const FaceInfo & fi,
                                                  PetscVectorReader & p_reader) const
{
  // Making sure the kernel knows which face we are on
  _p_diffusion_kernel->setupFaceData(&fi);

  // We are setting this to 1.0 because we don't want to multiply the kernel contributions
  // with the surface area yet. The surface area will be factored in in the advection kernels.
  // A provider whose pressure-correction fluxes are integrated takes the face measure instead.
  _p_diffusion_kernel->setCurrentFaceArea(pressureCorrectionFluxIsIntegrated() ? NS::faceMeasure(fi)
                                                                               : 1.0);

  Real p_grad_flux = 0.0;
  if (_p->isInternalFace(fi))
  {
    const auto & elem_info = *fi.elemInfo();
    const auto & neighbor_info = *fi.neighborInfo();

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
    const auto elem_rhs_contribution = _p_diffusion_kernel->computeElemRightHandSideContribution();

    // Compute the face flux from the matrix and right hand side contributions
    p_grad_flux = (p_neighbor_value * neighbor_matrix_contribution +
                   p_elem_value * elem_matrix_contribution) -
                  elem_rhs_contribution;
  }
  else if (auto * bc_pointer = _p->getBoundaryCondition(*fi.boundaryIDs().begin()))
  {
    mooseAssert(fi.boundaryIDs().size() == 1, "We should only have one boundary on every face.");
    bc_pointer->setupFaceData(
        &fi, fi.faceType(std::make_pair(_p->number(), _global_pressure_system_number)));

    const auto time_arg = Moose::currentState();
    const ElemInfo & elem_info =
        hasBlocks(fi.elemPtr()->subdomain_id()) ? *fi.elemInfo() : *fi.neighborInfo();
    const auto p_elem_value = _p->getElemValue(elem_info, time_arg);
    const auto matrix_contribution =
        _p_diffusion_kernel->computeBoundaryMatrixContribution(*bc_pointer);
    const auto rhs_contribution = _p_diffusion_kernel->computeBoundaryRHSContribution(*bc_pointer);

    // On the boundary, only the element side has a contribution
    p_grad_flux = p_elem_value * matrix_contribution - rhs_contribution;
  }

  return p_grad_flux;
}

void
RhieChowMassFlux::computeCellVelocity()
{
  updateCellVelocity(pressureGradientComponents());
}

void
RhieChowMassFlux::addMomentumPredictorExplicitForcing(const unsigned int /*system_i*/,
                                                      NumericVector<Number> & /*rhs*/) const
{
}

void
RhieChowMassFlux::computePredictorOperator(const unsigned int system_i,
                                           NumericVector<Number> & HbyA,
                                           NumericVector<Number> & Ainv)
{
  computePredictorOperatorBase(system_i, HbyA, Ainv);

  // Unfortunately, the pressure forces are included in the momentum RHS
  // so we have to correct them back using the same coupling gradient that
  // assembled the momentum pressure source.
  auto working_vector = _momentum_implicit_systems[system_i]->current_local_solution->zero_clone();
  auto * working_vector_petsc = dynamic_cast<PetscVector<Number> *>(working_vector.get());
  mooseAssert(working_vector_petsc,
              "The vectors used in the RhieChowMassFlux objects need to be convertable "
              "to PetscVectors!");
  working_vector_petsc->pointwise_mult(*_grad_p_current[system_i], *_cell_fluid_volumes);
  HbyA.add(-1.0, *working_vector_petsc);
}

void
RhieChowMassFlux::computePredictorOperatorBase(const unsigned int system_i,
                                               NumericVector<Number> & base_raw,
                                               NumericVector<Number> & diagonal_raw,
                                               const NumericVector<Number> * rhs_override) const
{
  mooseAssert(system_i < _momentum_implicit_systems.size() && _momentum_implicit_systems[system_i],
              "The requested momentum component is not linked to RhieChowMassFlux.");

  auto * momentum_system = _momentum_implicit_systems[system_i];
  auto * mmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system->matrix);
  mooseAssert(mmat,
              "The matrices used in the RhieChowMassFlux objects need to be convertible to "
              "PetscMatrix.");

  const NumericVector<Number> & rhs = rhs_override ? *rhs_override : *(momentum_system->rhs);
  const NumericVector<Number> & solution = *(momentum_system->solution);

  auto diagonal_parallel = solution.zero_clone();
  diagonal_parallel->close();
  mmat->get_diagonal(*diagonal_parallel);
  diagonal_parallel->close();
  diagonal_raw = *diagonal_parallel;
  diagonal_raw.close();

  auto working_vector = solution.zero_clone();
  working_vector->close();
  auto * working_vector_petsc = dynamic_cast<PetscVector<Number> *>(working_vector.get());
  mooseAssert(working_vector_petsc,
              "The vectors used in the RhieChowMassFlux objects need to be convertible to "
              "PetscVectors.");

  auto base_parallel = solution.zero_clone();
  base_parallel->close();
  mmat->vector_mult(*base_parallel, solution);
  base_parallel->close();
  working_vector_petsc->pointwise_mult(*diagonal_parallel, solution);
  working_vector_petsc->close();
  base_parallel->add(-1.0, *working_vector_petsc);
  base_parallel->close();
  base_parallel->add(-1.0, rhs);
  base_parallel->close();
  base_raw = *base_parallel;
  base_raw.close();

  mooseAssert(base_raw.first_local_index() ==
                      momentum_system->current_local_solution->first_local_index() &&
                  base_raw.last_local_index() ==
                      momentum_system->current_local_solution->last_local_index(),
              "Predictor operator cache must use the current-local vector layout.");
}

Real
RhieChowMassFlux::boundaryVelocityValue(const FaceInfo & fi,
                                        const unsigned int component,
                                        const Moose::StateArg & time_arg) const
{
  mooseAssert(component < _vel.size(), "Velocity component index out of range.");
  mooseAssert(!_vel[component]->isInternalFace(fi),
              "boundaryVelocityValue should only be called on boundary faces.");

  LinearFVBoundaryCondition * dirichlet_bc = nullptr;
  for (const auto bnd_id : fi.boundaryIDs())
    if (auto * const bc_pointer = _vel[component]->getBoundaryCondition(bnd_id))
      if (dynamic_cast<LinearFVAdvectionDiffusionFunctorDirichletBC *>(bc_pointer))
      {
        // Two Dirichlet conditions on one face would prescribe conflicting velocities.
        mooseAssert(!dirichlet_bc,
                    "Face " + std::to_string(fi.id()) +
                        " carries more than one Dirichlet velocity boundary condition.");
        dirichlet_bc = bc_pointer;
      }

  if (dirichlet_bc)
  {
    dirichlet_bc->setupFaceData(
        &fi,
        fi.faceType(
            std::make_pair(_vel[component]->number(), _global_momentum_system_numbers[component])));
    return dirichlet_bc->computeBoundaryValue();
  }

  return MetaPhysicL::raw_value((*_vel[component])(makeCDFace(fi), time_arg));
}

bool
RhieChowMassFlux::useConstrainedBoundaryPredictorState(const FaceInfo & fi) const
{
  if (_vel[0]->isInternalFace(fi))
    return false;

  // A LinearFVPressureFluxBC already enforces the prescribed boundary mass flux, so reconstructing
  // the predictor state from the Dirichlet velocity there would impose it twice. Those faces take
  // the one-term boundary expansion instead.
  if (dynamic_cast<const LinearFVPressureFluxBC *>(NS::linearFVBoundaryCondition(*_p, fi)))
    return false;

  return _vel[0]->isDirichletBoundaryFace(fi);
}

const ElemInfo &
RhieChowMassFlux::boundaryElemInfo(const FaceInfo & fi) const
{
  mooseAssert(!_vel[0]->isInternalFace(fi),
              "boundaryElemInfo should only be called on boundary faces.");
  // The velocity shares our blocks (checkBlocks), so its face type names the flow side
  return NS::linearFVFaceSideElemInfo(
      fi, fi.faceType(std::make_pair(_vel[0]->number(), _vel[0]->sys().number())));
}

Real
RhieChowMassFlux::boundaryNormalMultiplier(const FaceInfo & fi) const
{
  mooseAssert(!_vel[0]->isInternalFace(fi),
              "boundaryNormalMultiplier should only be called on boundary faces.");
  return NS::linearFVBoundaryNormalMultiplier(
      fi.faceType(std::make_pair(_vel[0]->number(), _vel[0]->sys().number())));
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

RhieChowMassFlux::FaceMomentumPredictorState
RhieChowMassFlux::buildMomentumPredictorFaceState(
    const FaceInfo & fi,
    const std::vector<PetscVectorReader> & hbya_reader,
    const std::vector<PetscVectorReader> & ainv_reader,
    const Moose::StateArg & time_arg) const
{
  using namespace Moose::FV;

  FaceMomentumPredictorState state;

  mooseAssert(hbya_reader.size() >= _dim, "HbyA readers must cover every momentum component.");
  mooseAssert(ainv_reader.size() >= _dim, "Ainv readers must cover every momentum component.");

  if (_vel[0]->isInternalFace(fi))
  {
    const auto & elem_info = *fi.elemInfo();
    const auto & neighbor_info = *fi.neighborInfo();
    const Real elem_rho = _rho(makeElemArg(fi.elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi.neighborPtr()), time_arg);

    interpolate(Moose::FV::InterpMethod::Average, state.face_rho, elem_rho, neighbor_rho, fi, true);

    for (const auto dim_i : make_range(_dim))
    {
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
      const auto neighbor_dof =
          neighbor_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
      interpolate(Moose::FV::InterpMethod::Average,
                  state.face_hbya(dim_i),
                  hbya_reader[dim_i](elem_dof),
                  hbya_reader[dim_i](neighbor_dof),
                  fi,
                  true);
      state.density_weighted_face_hbya(dim_i) = state.face_rho * state.face_hbya(dim_i);

      interpolate(_pressure_diffusion_interp_method,
                  state.face_density_weighted_ainv(dim_i),
                  elem_rho * ainv_reader[dim_i](elem_dof),
                  neighbor_rho * ainv_reader[dim_i](neighbor_dof),
                  fi,
                  true);
    }

    return state;
  }

  const Real boundary_normal_multiplier = boundaryNormalMultiplier(fi);
  const ElemInfo & elem_info = boundaryElemInfo(fi);
  const Moose::FaceArg boundary_face = makeCDFace(fi);
  const bool use_constrained_boundary_state = useConstrainedBoundaryPredictorState(fi);

  if (use_constrained_boundary_state)
  {
    state.face_rho = _rho(boundary_face, Moose::currentState());
    for (const auto dim_i : make_range(_dim))
    {
      state.face_hbya(dim_i) =
          -boundary_normal_multiplier * boundaryVelocityValue(fi, dim_i, time_arg);
      state.density_weighted_face_hbya(dim_i) = state.face_rho * state.face_hbya(dim_i);
    }
  }
  else
  {
    state.face_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
    for (const auto dim_i : make_range(_dim))
    {
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
      state.face_hbya(dim_i) = boundary_normal_multiplier * hbya_reader[dim_i](elem_dof);
      state.density_weighted_face_hbya(dim_i) = state.face_rho * state.face_hbya(dim_i);
    }
  }

  const Real elem_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
  for (const auto dim_i : make_range(_dim))
  {
    const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
    state.face_density_weighted_ainv(dim_i) = elem_rho * ainv_reader[dim_i](elem_dof);
  }

  return state;
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

  // We loop through the faces and populate the coupling fields (face H/A and 1/A)
  for (auto & fi : _flow_face_info)
  {
    const auto state = buildMomentumPredictorFaceState(*fi, hbya_reader, ainv_reader, time_arg);
    _Ainv[fi->id()] = state.face_density_weighted_ainv;
    // Lastly, we populate the face flux resulted by H/A
    _HbyA_flux[fi->id()] = state.density_weighted_face_hbya * fi->normal();
  }
}

void
RhieChowMassFlux::computeHbyA(bool verbose)
{
  if (verbose)
  {
    _console << "************************************" << std::endl;
    _console << "Computing HbyA" << std::endl;
    _console << "************************************" << std::endl;
  }
  mooseAssert(_momentum_implicit_systems.size() && _momentum_implicit_systems[0],
              "The momentum system shall be linked before calling this function!");

  mooseAssert(!_grad_p_current.empty(),
              "A coupling pressure-gradient snapshot must exist before computing H/A.");
  mooseAssert(_grad_p_current.size() == _momentum_implicit_systems.size(),
              "The coupling pressure-gradient snapshot must have one component per momentum "
              "system.");

#ifndef NDEBUG
  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    const auto & snapshot = *_grad_p_current[system_i];
    const auto & momentum_layout = *_momentum_implicit_systems[system_i]->current_local_solution;
    mooseAssert(snapshot.size() == momentum_layout.size() &&
                    snapshot.local_size() == momentum_layout.local_size() &&
                    snapshot.first_local_index() == momentum_layout.first_local_index() &&
                    snapshot.last_local_index() == momentum_layout.last_local_index(),
                "The coupling pressure-gradient snapshot layout must match its momentum system.");
  }
#endif

  const auto & coupling_pressure_gradient = _grad_p_current;
  _pressure_equation_flux_valid = false;

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

    if (verbose)
    {
      _console << "Velocity solution in H(u)" << std::endl;
      solution.print();
    }

    _Ainv_raw.push_back(current_local_solution.zero_clone());
    _HbyA_raw.push_back(current_local_solution.zero_clone());
    NumericVector<Number> & Ainv = *(_Ainv_raw.back());
    NumericVector<Number> & HbyA = *(_HbyA_raw.back());

    // H(u) - b and the diagonal A, formed the way this provider's predictor was assembled
    computePredictorOperator(system_i, HbyA, Ainv);

    // We create a working vector to ease some of the operations.
    auto working_vector = momentum_system->current_local_solution->zero_clone();
    PetscVector<Number> * working_vector_petsc =
        dynamic_cast<PetscVector<Number> *>(working_vector.get());
    mooseAssert(working_vector_petsc,
                "The vectors used in the RhieChowMassFlux objects need to be convertable "
                "to PetscVectors!");

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
      working_vector_petsc->pointwise_mult(*working_vector_petsc, *_cell_fluid_volumes);
      HbyA.add(-1.0, *working_vector_petsc);

      // Correct Ainv
      Ainv = *Ainv_full_old;
    }

    Ainv.pointwise_mult(Ainv, *_cell_fluid_volumes);

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

const std::vector<NumericVector<Number> *> &
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

FVReconstructedPressureGradient &
RhieChowMassFlux::reconstructedGradientMethod()
{
  auto & method = _fe_problem.getFVGradientMethod(pressureGradientField().method().name(), _tid);
  mooseAssert(&method == &pressureGradientField().method(),
              "The registered writable gradient method must match the pressure-gradient reader.");
  return dynamic_cast<FVReconstructedPressureGradient &>(method);
}

void
RhieChowMassFlux::checkReconstructedPressureGradientCompatibility() const
{
  if (!_pressure_system || !_pressure_gradient_field || !_base_pressure_gradient_field ||
      !_p_diffusion_kernel)
    mooseError("RhieChowMassFlux '",
               name(),
               "' is missing a required pressure system, pressure-gradient field, base-gradient "
               "field, or pressure-correction diffusion kernel for reconstructed pressure "
               "gradients.");

  reconstructedGradientMethod().validateSetup(*this);

  if (_momentum_systems.size() != _dim || _momentum_implicit_systems.size() != _dim ||
      _global_momentum_system_numbers.size() != _dim)
    mooseError("RhieChowMassFlux '",
               name(),
               "' requires one linked momentum system per spatial component for reconstructed "
               "pressure gradients.");

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
    if (!momentum_system || !_momentum_implicit_systems[system_i])
      mooseError("RhieChowMassFlux '",
                 name(),
                 "' has an invalid momentum-system layout for component ",
                 system_i,
                 ".");

    if (momentum_system->nVariables() != 1)
      mooseError("FVReconstructedPressureGradient assumes the pressure and momentum systems each "
                 "have exactly one variable so their DOF indices can be used in vector operations. "
                 "Momentum system '",
                 momentum_system->name(),
                 "' has variables: ",
                 Moose::stringify(momentum_system->getVariableNames()));
  }
}

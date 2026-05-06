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
#include "LinearFVBoundaryCondition.h"
#include "LinearFVPressureCorrectionDiffusion.h"
#include "LinearFVPressureFluxBC.h"
#include "LinearFVPressureInletOutletVelocityBC.h"
#include "LinearFVPressureSymmetryBC.h"
#include "Function.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

// libMesh includes
#include "libmesh/fe.h"
#include "libmesh/fe_type.h"
#include "libmesh/mesh_base.h"
#include "libmesh/elem_range.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/quadrature_gauss.h"

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
  params.addParam<std::vector<std::vector<std::string>>>(
      "body_force_kernel_names",
      {},
      "The body force kernel names."
      "this double vector would have size index_x_dim: 'f1x f2x; f1y f2y; f1z f2z'");

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
      "use_cached_momentum_predictor_operator",
      false,
      "Whether to use a cached assembled/relaxed momentum predictor operator as the starting "
      "point for HbyA/Ainv construction instead of rebuilding that base state from the live "
      "system on demand.");
  params.addParam<bool>(
      "split_momentum_predictor_operator",
      false,
      "Whether the assembled momentum predictor operator already excludes explicit pressure-"
      "gradient and body-force terms, so HbyA should use that operator directly instead of "
      "subtracting those terms back out.");
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
    _phiHbyA_flux(_moose_mesh, blockIDs(), "phiHbyA_flux"),
    _phig_flux(_moose_mesh, blockIDs(), "phig_flux"),
    _Ainv(_moose_mesh, blockIDs(), "Ainv"),
    _face_mass_flux(
        declareRestartableData<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>>(
            "face_flux", _moose_mesh, blockIDs(), "face_values")),
    _pressure_equation_flux(_moose_mesh, blockIDs(), "pressure_equation_flux"),
    _pressure_boundary_normal_gradient(
        _moose_mesh, blockIDs(), "pressure_boundary_normal_gradient"),
    _boundary_velocity_face_values(_dim),
    _use_cached_momentum_predictor_operator(
        getParam<bool>("use_cached_momentum_predictor_operator")),
    _split_momentum_predictor_operator(getParam<bool>("split_momentum_predictor_operator")),
    _body_force_kernel_names(
        getParam<std::vector<std::vector<std::string>>>("body_force_kernel_names")),
    _rho(getFunctor<Real>(NS::density)),
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
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("Ainv", _Ainv, tid);
    UserObject::_subproblem.addFunctor("HbyA", _HbyA_flux, tid);
    UserObject::_subproblem.addFunctor("phiHbyA", _phiHbyA_flux, tid);
    UserObject::_subproblem.addFunctor("phig", _phig_flux, tid);
    UserObject::_subproblem.addFunctor("pressure_equation_flux", _pressure_equation_flux, tid);
    UserObject::_subproblem.addFunctor(
        "pressure_boundary_normal_gradient", _pressure_boundary_normal_gradient, tid);
  }

  if (!dynamic_cast<SIMPLE *>(getMooseApp().getExecutioner()) &&
      !dynamic_cast<PIMPLE *>(getMooseApp().getExecutioner()))
    mooseError(this->name(),
               " should only be used with a linear segregated thermal-hydraulics solver!");
}

void
RhieChowMassFlux::linkMomentumPressureSystems(
    const std::vector<LinearSystem *> & momentum_systems,
    const LinearSystem & pressure_system,
    const std::vector<unsigned int> & momentum_system_numbers)
{
  _momentum_systems = momentum_systems;
  _momentum_system_numbers = momentum_system_numbers;
  _pressure_system = &pressure_system;
  _global_pressure_system_number = _pressure_system->number();

  _momentum_implicit_systems.clear();
  for (auto & system : _momentum_systems)
  {
    _global_momentum_system_numbers.push_back(system->number());
    _momentum_implicit_systems.push_back(dynamic_cast<LinearImplicitSystem *>(&system->system()));
  }

  setupMeshInformation();
}

void
RhieChowMassFlux::meshChanged()
{
  _HbyA_flux.clear();
  _phiHbyA_flux.clear();
  _phig_flux.clear();
  _Ainv.clear();
  _face_mass_flux.clear();
  _pressure_equation_flux.clear();
  _pressure_boundary_normal_gradient.clear();
  _pressure_predictor_flux_adjustment.clear();
  _pressure_predictor_base_flux.clear();
  _pressure_equation_flux_valid = false;
  _pressure_boundary_normal_gradient_valid = false;
  _pressure_predictor_face_state_valid = false;
  for (auto & component_face_values : _boundary_velocity_face_values)
    component_face_values.clear();
  _velocity_boundary_state_valid = false;
  clearMomentumPredictorOperatorCache();
  setupMeshInformation();
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
               "The provided diffusion kernel should be of type "
               "LinearFVPressureCorrectionDiffusion.");

  // We fetch the body forces kernel to ensure that the face flux correction
  // is accurate.

  // Check if components match the dimension.

  if (!_body_force_kernel_names.empty())
  {
    if (_body_force_kernel_names.size() != _dim)
      paramError("body_force_kernel_names",
                 "The dimension of the body force vector does not match the problem dimension.");

    _body_force_kernels.resize(_dim);

    for (const auto dim_i : make_range(_dim))
      for (const auto & force_name : _body_force_kernel_names[dim_i])
      {
        std::vector<LinearFVElementalKernel *> temp_storage;
        auto base_query_force = _fe_problem.theWarehouse()
                                    .query()
                                    .template condition<AttribThread>(_tid)
                                    .template condition<AttribSysNum>(_vel[dim_i]->sys().number())
                                    .template condition<AttribSystem>("LinearFVElementalKernel")
                                    .template condition<AttribName>(force_name)
                                    .queryInto(temp_storage);
        if (temp_storage.size() != 1)
          paramError("body_force_kernel_names",
                     "The kernel with the given name: " + force_name +
                         " could not be found or multiple instances were identified.");
        _body_force_kernels[dim_i].push_back(temp_storage[0]);
      }
  }
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

  for (const auto * fi : _flow_face_info)
  {
    _pressure_boundary_normal_gradient[fi->id()] = 0.0;
    _pressure_predictor_flux_adjustment[fi->id()] = 0.0;
    _pressure_predictor_base_flux[fi->id()] = 0.0;
  }
}

void
RhieChowMassFlux::initialize()
{
  for (const auto & pair : _HbyA_flux)
    _HbyA_flux[pair.first] = 0;

  for (const auto & pair : _phiHbyA_flux)
    _phiHbyA_flux[pair.first] = 0;

  for (const auto & pair : _phig_flux)
    _phig_flux[pair.first] = 0;

  for (const auto & pair : _Ainv)
    _Ainv[pair.first] = 0;

  for (const auto & pair : _pressure_equation_flux)
    _pressure_equation_flux[pair.first] = 0;

  for (const auto & pair : _pressure_boundary_normal_gradient)
    _pressure_boundary_normal_gradient[pair.first] = 0;

  _pressure_predictor_flux_adjustment.clear();
  for (const auto * fi : _flow_face_info)
    _pressure_predictor_flux_adjustment[fi->id()] = 0.0;

  _pressure_equation_flux_valid = false;
  _pressure_boundary_normal_gradient_valid = false;
  _pressure_predictor_face_state_valid = false;
  for (auto & component_face_values : _boundary_velocity_face_values)
    component_face_values.clear();
  _velocity_boundary_state_valid = false;
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
                                        boundaryVelocityValue(fi, dim_i, time_arg);
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

Real
RhieChowMassFlux::boundaryMassFluxImbalance() const
{
  Real imbalance = 0.0;

  for (const auto * fi : _flow_face_info)
    if (!_vel[0]->isInternalFace(*fi))
      imbalance += libmesh_map_find(_face_mass_flux, fi->id());

  return imbalance;
}

Real
RhieChowMassFlux::maxBoundaryMassFluxMagnitude() const
{
  Real max_boundary_flux = 0.0;

  for (const auto * fi : _flow_face_info)
    if (!_vel[0]->isInternalFace(*fi))
      max_boundary_flux =
          std::max(max_boundary_flux, std::abs(libmesh_map_find(_face_mass_flux, fi->id())));

  return max_boundary_flux;
}

Real
RhieChowMassFlux::faceMassFluxL2Norm() const
{
  Real squared_sum = 0.0;

  for (const auto * fi : _flow_face_info)
  {
    const Real flux = libmesh_map_find(_face_mass_flux, fi->id());
    squared_sum += flux * flux;
  }

  return std::sqrt(squared_sum);
}

Real
RhieChowMassFlux::maxCourant(const Real dt) const
{
  if (dt <= 0.0)
    return 0.0;

  std::unordered_map<dof_id_type, Real> cell_flux_sum;
  cell_flux_sum.reserve(_flow_face_info.size());

  for (const auto * fi : _flow_face_info)
  {
    if (!fi)
      continue;

    const Real face_measure = fi->faceArea() * fi->faceCoord();
    const Real volumetric_flux = std::abs(getVolumetricFaceFlux(*fi)) * face_measure;

    if (fi->elemPtr() && hasBlocks(fi->elemPtr()->subdomain_id()))
      cell_flux_sum[fi->elemPtr()->id()] += volumetric_flux;

    if (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id()))
      cell_flux_sum[fi->neighborPtr()->id()] += volumetric_flux;
  }

  Real max_courant = 0.0;
  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const Real volume = elem_info->volume() * elem_info->coordFactor();
    if (volume <= libMesh::TOLERANCE)
      continue;

    const auto it = cell_flux_sum.find(elem_info->elem()->id());
    if (it == cell_flux_sum.end())
      continue;

    max_courant = std::max(max_courant, 0.5 * dt * it->second / volume);
  }

  return max_courant;
}

RhieChowMassFlux::FaceFluxConsistencyAudit
RhieChowMassFlux::faceFluxConsistencyAudit() const
{
  using namespace Moose::FV;

  FaceFluxConsistencyAudit audit;
  const auto time_arg = Moose::currentState();

  Real squared_sum = 0.0;
  Real internal_squared_sum = 0.0;
  Real boundary_squared_sum = 0.0;

  for (const auto * fi : _flow_face_info)
  {
    Real reconstructed_flux = 0.0;

    if (_vel[0]->isInternalFace(*fi))
    {
      RealVectorValue face_velocity;
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();

      for (const auto dim_i : index_range(_vel))
        interpolate(InterpMethod::Average,
                    face_velocity(dim_i),
                    _vel[dim_i]->getElemValue(elem_info, time_arg),
                    _vel[dim_i]->getElemValue(neighbor_info, time_arg),
                    *fi,
                    true);

      reconstructed_flux = face_velocity * fi->normal();
    }
    else
      reconstructed_flux = boundaryVolumetricFluxTarget(fi, time_arg);

    const Real stored_flux = getVolumetricFaceFlux(*fi);
    const Real mismatch = stored_flux - reconstructed_flux;
    const Real abs_mismatch = std::abs(mismatch);

    squared_sum += mismatch * mismatch;

    const bool is_boundary = !_vel[0]->isInternalFace(*fi);
    if (is_boundary)
    {
      boundary_squared_sum += mismatch * mismatch;
      audit.max_abs_boundary_mismatch =
          std::max(audit.max_abs_boundary_mismatch, abs_mismatch);
    }
    else
    {
      internal_squared_sum += mismatch * mismatch;
      audit.max_abs_internal_mismatch =
          std::max(audit.max_abs_internal_mismatch, abs_mismatch);
    }

    if (!audit.has_worst_face || abs_mismatch > audit.max_abs_mismatch)
    {
      audit.has_worst_face = true;
      audit.max_abs_mismatch = abs_mismatch;
      audit.worst_face_id = fi->id();
      audit.worst_face_is_boundary = is_boundary;
      audit.worst_face_centroid = fi->faceCentroid();
      audit.worst_face_normal = fi->normal();
    }
  }

  audit.l2_norm = std::sqrt(squared_sum);
  audit.internal_l2_norm = std::sqrt(internal_squared_sum);
  audit.boundary_l2_norm = std::sqrt(boundary_squared_sum);
  return audit;
}

void
RhieChowMassFlux::cachePressureEquationFlux()
{
  using namespace Moose::FV;

  // Petsc vector reader to make the repeated reading from the vector faster
  PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);

  // We loop through the faces and compute the exact discrete pressure-equation face flux
  // from the solved pressure field.
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
      const auto elem_dof = elem_info.dofIndices()[_global_pressure_system_number][0];
      const auto p_elem_value = p_reader(elem_dof);
      const auto matrix_contribution =
          _p_diffusion_kernel->computeBoundaryMatrixContribution(*bc_pointer);
      const auto rhs_contribution =
          _p_diffusion_kernel->computeBoundaryRHSContribution(*bc_pointer);

      // On the boundary, only the element side has a contribution
      p_grad_flux = (p_elem_value * matrix_contribution - rhs_contribution);
    }
    _pressure_equation_flux[fi->id()] = p_grad_flux;
  }

  _pressure_equation_flux_valid = true;
}

Real
RhieChowMassFlux::exactInternalPressureEquationFlux(const FaceInfo & fi,
                                                    const Function & exact_pressure) const
{
  mooseAssert(_p->isInternalFace(fi),
              "exactInternalPressureEquationFlux is only defined on internal faces.");

  auto cell_average = [this, &exact_pressure](const Elem * elem)
  {
    std::unique_ptr<FEBase> fe(FEBase::build(elem->dim(), FEType(CONSTANT, MONOMIAL)));
    QGauss qrule(elem->dim(), FIFTH);
    fe->attach_quadrature_rule(&qrule);
    fe->get_xyz();
    fe->get_JxW();
    fe->reinit(elem);

    const auto & q_points = fe->get_xyz();
    const auto & JxW = fe->get_JxW();

    Real integral = 0.0;
    Real volume = 0.0;
    for (const auto qp : index_range(q_points))
    {
      integral += exact_pressure.value(_t, q_points[qp]) * JxW[qp];
      volume += JxW[qp];
    }

    return volume > 0.0 ? integral / volume : exact_pressure.value(_t, elem->vertex_average());
  };

  _p_diffusion_kernel->setupFaceData(&fi);
  _p_diffusion_kernel->setCurrentFaceArea(1.0);

  const Real p_elem = cell_average(fi.elemPtr());
  const Real p_neighbor = cell_average(fi.neighborPtr());

  const Real elem_matrix_contribution = _p_diffusion_kernel->computeElemMatrixContribution();
  const Real neighbor_matrix_contribution =
      _p_diffusion_kernel->computeNeighborMatrixContribution();
  const Real elem_rhs_contribution = _p_diffusion_kernel->computeElemRightHandSideContribution();

  return p_neighbor * neighbor_matrix_contribution + p_elem * elem_matrix_contribution -
         elem_rhs_contribution;
}

Real
RhieChowMassFlux::storedPressureEquationFlux(const FaceInfo & fi) const
{
  return libmesh_map_find(_pressure_equation_flux, fi.id());
}

Real
RhieChowMassFlux::pressurePredictorFlux(const FaceInfo * fi) const
{
  return libmesh_map_find(_HbyA_flux, fi->id());
}

void
RhieChowMassFlux::updatePressurePredictorFaceState()
{
  for (const auto * fi : _flow_face_info)
  {
    const auto face_id = fi->id();
    _phig_flux[face_id] = 0.0;
    _pressure_predictor_base_flux[face_id] = pressurePredictorFlux(fi);
    _phiHbyA_flux[face_id] = _pressure_predictor_base_flux[face_id];
  }

  _pressure_predictor_face_state_valid = true;
}

void
RhieChowMassFlux::computeFaceMassFlux()
{
  if (!_pressure_predictor_face_state_valid)
    updatePressurePredictorFaceState();

  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  for (auto & fi : _flow_face_info)
    _face_mass_flux[fi->id()] = -_phiHbyA_flux[fi->id()] + _pressure_equation_flux[fi->id()];
}

void
RhieChowMassFlux::computeCellVelocity()
{
  auto & pressure_gradient = _pressure_system->linearFVGradientContainer();

  // We set the dof value in the solution vector the same logic applies:
  // u_C = -(H/A)_C - (1/A)_C*grad(p)_C where C is the cell index
  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    auto working_vector = _Ainv_raw[system_i]->clone();
    working_vector->pointwise_mult(*working_vector, *pressure_gradient[system_i]);
    working_vector->add(*_HbyA_raw[system_i]);
    working_vector->scale(-1.0);
    (*_momentum_implicit_systems[system_i]->solution) = *working_vector;
    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }

  _velocity_boundary_state_valid = false;
}

Real
RhieChowMassFlux::cellHbyARaw(const unsigned int system_i, const dof_id_type dof) const
{
  mooseAssert(system_i < _HbyA_raw.size() && _HbyA_raw[system_i],
              "Requested HbyA component is not available.");
  return (*_HbyA_raw[system_i])(dof);
}

Real
RhieChowMassFlux::cellAinvRaw(const unsigned int system_i, const dof_id_type dof) const
{
  mooseAssert(system_i < _Ainv_raw.size() && _Ainv_raw[system_i],
              "Requested Ainv component is not available.");
  return (*_Ainv_raw[system_i])(dof);
}

void
RhieChowMassFlux::clearMomentumPredictorOperatorCache()
{
  _cached_predictor_operator_base_raw.clear();
  _cached_predictor_diagonal_raw.clear();
  _cached_predictor_explicit_force_raw.clear();
  _cached_predictor_body_force_raw.clear();
  _cached_predictor_operator_valid = false;
}

void
RhieChowMassFlux::addMomentumPredictorExplicitForcing(const unsigned int /*system_i*/,
                                                      NumericVector<Number> & /*rhs*/) const
{
}

void
RhieChowMassFlux::addMomentumPredictorBodyForceForcing(const unsigned int /*system_i*/,
                                                       NumericVector<Number> & /*rhs*/) const
{
}

bool
RhieChowMassFlux::canUseCachedMomentumPredictorOperator() const
{
  if (!_use_cached_momentum_predictor_operator || !_cached_predictor_operator_valid)
    return false;

  if (_cached_predictor_operator_base_raw.size() != _momentum_systems.size() ||
      _cached_predictor_diagonal_raw.size() != _momentum_systems.size())
    return false;

  if (_split_momentum_predictor_operator &&
      _cached_predictor_explicit_force_raw.size() != _momentum_systems.size())
    return false;
  if (_split_momentum_predictor_operator &&
      _cached_predictor_body_force_raw.size() != _momentum_systems.size())
    return false;

  return std::all_of(_cached_predictor_operator_base_raw.begin(),
                     _cached_predictor_operator_base_raw.end(),
                     [](const auto & vec) { return static_cast<bool>(vec); }) &&
         std::all_of(_cached_predictor_diagonal_raw.begin(),
                     _cached_predictor_diagonal_raw.end(),
                     [](const auto & vec) { return static_cast<bool>(vec); }) &&
         (!_split_momentum_predictor_operator ||
          std::all_of(_cached_predictor_explicit_force_raw.begin(),
                      _cached_predictor_explicit_force_raw.end(),
                      [](const auto & vec) { return static_cast<bool>(vec); })) &&
         (!_split_momentum_predictor_operator ||
          std::all_of(_cached_predictor_body_force_raw.begin(),
                      _cached_predictor_body_force_raw.end(),
                      [](const auto & vec) { return static_cast<bool>(vec); }));
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
  NumericVector<Number> & solution = *(momentum_system->current_local_solution);

  mmat->get_diagonal(diagonal_raw);

  auto working_vector = solution.zero_clone();
  auto * working_vector_petsc = dynamic_cast<PetscVector<Number> *>(working_vector.get());
  mooseAssert(working_vector_petsc,
              "The vectors used in the RhieChowMassFlux objects need to be convertible to "
              "PetscVectors.");

  mmat->vector_mult(base_raw, solution);
  working_vector_petsc->pointwise_mult(diagonal_raw, solution);
  base_raw.add(-1.0, *working_vector_petsc);
  base_raw.add(-1.0, rhs);
}

void
RhieChowMassFlux::cacheMomentumPredictorOperator(const unsigned int system_i,
                                                 const NumericVector<Number> * rhs_override,
                                                 const NumericVector<Number> * explicit_force,
                                                 const NumericVector<Number> * body_force)
{
  if (!_use_cached_momentum_predictor_operator)
    return;

  mooseAssert(system_i < _momentum_implicit_systems.size() && _momentum_implicit_systems[system_i],
              "The requested momentum component is not linked to RhieChowMassFlux.");

  if (_cached_predictor_operator_base_raw.size() != _momentum_systems.size())
    _cached_predictor_operator_base_raw.resize(_momentum_systems.size());
  if (_cached_predictor_diagonal_raw.size() != _momentum_systems.size())
    _cached_predictor_diagonal_raw.resize(_momentum_systems.size());
  if (_cached_predictor_explicit_force_raw.size() != _momentum_systems.size())
    _cached_predictor_explicit_force_raw.resize(_momentum_systems.size());
  if (_cached_predictor_body_force_raw.size() != _momentum_systems.size())
    _cached_predictor_body_force_raw.resize(_momentum_systems.size());

  auto * momentum_system = _momentum_implicit_systems[system_i];
  NumericVector<Number> & current_local_solution = *(momentum_system->current_local_solution);

  _cached_predictor_operator_base_raw[system_i] = current_local_solution.zero_clone();
  _cached_predictor_diagonal_raw[system_i] = current_local_solution.zero_clone();

  computePredictorOperatorBase(
      system_i,
      *_cached_predictor_operator_base_raw[system_i],
      *_cached_predictor_diagonal_raw[system_i],
      rhs_override);

  if (_split_momentum_predictor_operator)
  {
    _cached_predictor_explicit_force_raw[system_i] = current_local_solution.zero_clone();
    if (explicit_force)
      *_cached_predictor_explicit_force_raw[system_i] = *explicit_force;
    else
      _cached_predictor_explicit_force_raw[system_i]->zero();
    _cached_predictor_explicit_force_raw[system_i]->close();

    _cached_predictor_body_force_raw[system_i] = current_local_solution.zero_clone();
    if (body_force)
      *_cached_predictor_body_force_raw[system_i] = *body_force;
    else
      _cached_predictor_body_force_raw[system_i]->zero();
    _cached_predictor_body_force_raw[system_i]->close();
  }
  else
  {
    _cached_predictor_explicit_force_raw[system_i].reset();
    _cached_predictor_body_force_raw[system_i].reset();
  }

  _cached_predictor_operator_valid =
      _cached_predictor_operator_base_raw.size() == _momentum_systems.size() &&
      _cached_predictor_diagonal_raw.size() == _momentum_systems.size() &&
      (!_split_momentum_predictor_operator ||
       (_cached_predictor_explicit_force_raw.size() == _momentum_systems.size() &&
        _cached_predictor_body_force_raw.size() == _momentum_systems.size())) &&
      std::all_of(_cached_predictor_operator_base_raw.begin(),
                  _cached_predictor_operator_base_raw.end(),
                  [](const auto & vec) { return static_cast<bool>(vec); }) &&
      std::all_of(_cached_predictor_diagonal_raw.begin(),
                  _cached_predictor_diagonal_raw.end(),
                  [](const auto & vec) { return static_cast<bool>(vec); }) &&
      (!_split_momentum_predictor_operator ||
       std::all_of(_cached_predictor_explicit_force_raw.begin(),
                   _cached_predictor_explicit_force_raw.end(),
                   [](const auto & vec) { return static_cast<bool>(vec); })) &&
      (!_split_momentum_predictor_operator ||
       std::all_of(_cached_predictor_body_force_raw.begin(),
                   _cached_predictor_body_force_raw.end(),
                   [](const auto & vec) { return static_cast<bool>(vec); }));
}

void
RhieChowMassFlux::cacheStartupPredictorDiagonal(const unsigned int system_i,
                                                const NumericVector<Number> & diagonal_raw)
{
  mooseAssert(system_i < _momentum_implicit_systems.size() && _momentum_implicit_systems[system_i],
              "The requested momentum component is not linked to RhieChowMassFlux.");

  if (_cached_predictor_diagonal_raw.size() != _momentum_systems.size())
    _cached_predictor_diagonal_raw.resize(_momentum_systems.size());

  NumericVector<Number> & current_local_solution =
      *(_momentum_implicit_systems[system_i]->current_local_solution);
  _cached_predictor_diagonal_raw[system_i] = current_local_solution.zero_clone();
  *(_cached_predictor_diagonal_raw[system_i]) = diagonal_raw;
  _cached_predictor_diagonal_raw[system_i]->close();
}

void
RhieChowMassFlux::updateVelocityBoundaryState()
{
  const auto time_arg = Moose::currentState();

  for (auto & component_face_values : _boundary_velocity_face_values)
    component_face_values.clear();

  for (const auto * fi : _flow_face_info)
  {
    if (_vel[0]->isInternalFace(*fi))
      continue;

    RealVectorValue density_times_velocity;
    const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
    const Elem * const boundary_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
    const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
    const Moose::FaceArg boundary_face{
        fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};
    const Real face_rho = _rho(boundary_face, time_arg);

    for (const auto component : index_range(_vel))
    {
      if (!fi->boundaryIDs().empty())
      {
        mooseAssert(fi->boundaryIDs().size() == 1,
                    "Expected at most one physical boundary id on a FV boundary face.");
      }

      if (!fi->boundaryIDs().empty() &&
          _vel[component]->getBoundaryCondition(*fi->boundaryIDs().begin()))
      {
        auto * bc_pointer = _vel[component]->getBoundaryCondition(*fi->boundaryIDs().begin());
        bc_pointer->setupFaceData(
            fi,
            fi->faceType(
                std::make_pair(_vel[component]->number(), _vel[component]->sys().number())));
        _boundary_velocity_face_values[component][fi->id()] = bc_pointer->computeBoundaryValue();
      }
      else
      {
        const ElemInfo & elem_info =
            hasBlocks(fi->elemPtr()->subdomain_id()) ? *fi->elemInfo() : *fi->neighborInfo();
        _boundary_velocity_face_values[component][fi->id()] =
            _vel[component]->getElemValue(elem_info, time_arg);
      }

      density_times_velocity(component) =
          boundary_normal_multiplier * face_rho * _boundary_velocity_face_values[component][fi->id()];
    }

    // Re-publish the boundary face mass flux from the refreshed patch state so
    // the next outer momentum predictor sees a boundary phi consistent with the
    // corrected boundary velocity values.
    _face_mass_flux[fi->id()] = density_times_velocity * fi->normal();
  }

  _velocity_boundary_state_valid = true;
}

Real
RhieChowMassFlux::boundaryVelocityValue(const FaceInfo * fi,
                                        const unsigned int component,
                                        const Moose::StateArg & time_arg) const
{
  mooseAssert(fi, "FaceInfo should not be null when evaluating a boundary velocity.");
  mooseAssert(component < _vel.size(), "Velocity component index out of range.");
  mooseAssert(!_vel[component]->isInternalFace(*fi),
              "boundaryVelocityValue should only be called on boundary faces.");

  if (_velocity_boundary_state_valid)
    if (const auto it = _boundary_velocity_face_values[component].find(fi->id());
        it != _boundary_velocity_face_values[component].end())
      return it->second;

  if (!fi->boundaryIDs().empty())
  {
    mooseAssert(fi->boundaryIDs().size() == 1,
                "Expected at most one physical boundary id on a FV boundary face.");

    if (auto * bc_pointer = _vel[component]->getBoundaryCondition(*fi->boundaryIDs().begin()))
    {
      bc_pointer->setupFaceData(
          fi,
          fi->faceType(
              std::make_pair(_vel[component]->number(), _vel[component]->sys().number())));
      return bc_pointer->computeBoundaryValue();
    }
  }

  const ElemInfo & elem_info =
      hasBlocks(fi->elemPtr()->subdomain_id()) ? *fi->elemInfo() : *fi->neighborInfo();
  return _vel[component]->getElemValue(elem_info, time_arg);
}

Real
RhieChowMassFlux::boundaryMassFluxTarget(const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  mooseAssert(fi && !_vel[0]->isInternalFace(*fi),
              "boundaryMassFluxTarget should only be called on boundary faces.");

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const boundary_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
  const Moose::FaceArg boundary_face{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};

  const Real face_rho = _rho(boundary_face, time_arg);
  RealVectorValue density_times_velocity;
  for (const auto component : index_range(_vel))
    density_times_velocity(component) =
        boundary_normal_multiplier * face_rho * boundaryVelocityValue(fi, component, time_arg);

  return density_times_velocity * fi->normal();
}

Real
RhieChowMassFlux::boundaryVolumetricFluxTarget(const FaceInfo * fi,
                                               const Moose::StateArg & time_arg) const
{
  mooseAssert(fi && !_vel[0]->isInternalFace(*fi),
              "boundaryVolumetricFluxTarget should only be called on boundary faces.");

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;

  RealVectorValue face_velocity;
  for (const auto component : index_range(_vel))
    face_velocity(component) =
        boundary_normal_multiplier * boundaryVelocityValue(fi, component, time_arg);

  return face_velocity * fi->normal();
}

Real
RhieChowMassFlux::boundaryNormalAinv(const FaceInfo * fi) const
{
  mooseAssert(fi && !_vel[0]->isInternalFace(*fi),
              "boundaryNormalAinv should only be called on boundary faces.");

  const auto & face_ainv = libmesh_map_find(_Ainv, fi->id());
  const auto & normal = fi->normal();

  Real normal_ainv = 0.0;
  for (const auto dim_i : make_range(_dim))
    normal_ainv += face_ainv(dim_i) * normal(dim_i) * normal(dim_i);

  return normal_ainv;
}

bool
RhieChowMassFlux::isAdjustablePressureBoundaryFace(const FaceInfo * fi) const
{
  if (!fi || _vel[0]->isInternalFace(*fi) || fi->boundaryIDs().empty())
    return false;

  mooseAssert(fi->boundaryIDs().size() == 1, "Expected a single boundary id on a FV boundary face.");
  if (const auto * bc_pointer = _p->getBoundaryCondition(*fi->boundaryIDs().begin()))
    return dynamic_cast<const LinearFVPressureFluxBC *>(bc_pointer) ||
           dynamic_cast<const LinearFVPressureSymmetryBC *>(bc_pointer);

  return false;
}

Real
RhieChowMassFlux::pressurePredictorFluxAdjustment(const FaceInfo * fi) const
{
  if (!fi)
    return 0.0;

  const auto it = _pressure_predictor_flux_adjustment.find(fi->id());
  return it == _pressure_predictor_flux_adjustment.end() ? 0.0 : it->second;
}

Real
RhieChowMassFlux::pressureBoundaryTargetFlux(const FaceInfo * fi,
                                             const Moose::StateArg & time_arg) const
{
  return boundaryMassFluxTarget(fi, time_arg);
}

Real
RhieChowMassFlux::pressureBoundaryNormalAinv(const FaceInfo * fi) const
{
  return boundaryNormalAinv(fi);
}

void
RhieChowMassFlux::updatePressureBoundaryNormalGradients(const bool apply_reference_adjustment)
{
  if (!_pressure_predictor_face_state_valid)
    updatePressurePredictorFaceState();

  const auto time_arg = Moose::currentState();
  Real max_boundary_normal_ainv = 0.0;

  for (const auto * fi : _flow_face_info)
  {
    _pressure_predictor_flux_adjustment[fi->id()] = 0.0;
    _phiHbyA_flux[fi->id()] = _pressure_predictor_base_flux[fi->id()];
    if (!_vel[0]->isInternalFace(*fi))
    {
      _pressure_boundary_normal_gradient[fi->id()] = 0.0;
      max_boundary_normal_ainv =
          std::max(max_boundary_normal_ainv, std::abs(pressureBoundaryNormalAinv(fi)));
    }
  }

  // Treat a boundary pressure coefficient as degenerate only relative to the scale of the
  // current boundary operator. An absolute libMesh::TOLERANCE cutoff is too aggressive for
  // high-density-ratio outlet faces, where valid normal Ainv values can be smaller than 1e-6.
  const Real degenerate_normal_ainv_tol =
      std::max(std::numeric_limits<Real>::min(),
               std::numeric_limits<Real>::epsilon() * std::max(1.0, max_boundary_normal_ainv));

  // Local adjustPhi analogue for the pressure-reference branch: shift the predictor source
  // on adjustable fixed-flux patches so the integrated predictor boundary flux matches the
  // boundary mass-flux target before the pressure solve.
  if (apply_reference_adjustment)
  {
    Real integrated_source_imbalance = 0.0;
    Real adjustable_measure = 0.0;

    for (const auto * fi : _flow_face_info)
      if (isAdjustablePressureBoundaryFace(fi))
      {
        const Real face_measure = fi->faceArea() * fi->faceCoord();
        integrated_source_imbalance +=
            (_pressure_predictor_base_flux[fi->id()] + pressureBoundaryTargetFlux(fi, time_arg)) *
            face_measure;
        adjustable_measure += face_measure;
      }

    if (adjustable_measure > libMesh::TOLERANCE)
    {
      const Real uniform_source_adjustment = -integrated_source_imbalance / adjustable_measure;
      for (const auto * fi : _flow_face_info)
        if (isAdjustablePressureBoundaryFace(fi))
        {
          _pressure_predictor_flux_adjustment[fi->id()] = uniform_source_adjustment;
          _phiHbyA_flux[fi->id()] = _pressure_predictor_base_flux[fi->id()] +
                                    uniform_source_adjustment;
        }
    }
  }

  for (const auto * fi : _flow_face_info)
  {
    if (_vel[0]->isInternalFace(*fi))
      continue;

    const Real normal_ainv = pressureBoundaryNormalAinv(fi);
    if (!std::isfinite(normal_ainv) || std::abs(normal_ainv) <= degenerate_normal_ainv_tol)
    {
      _pressure_boundary_normal_gradient[fi->id()] = 0.0;
      continue;
    }

    const Real required_pressure_flux =
        _phiHbyA_flux[fi->id()] + pressureBoundaryTargetFlux(fi, time_arg);
    _pressure_boundary_normal_gradient[fi->id()] = -required_pressure_flux / normal_ainv;
  }

  _pressure_boundary_normal_gradient_valid = true;
}

void
RhieChowMassFlux::auditPressureBoundaryGradientState(const std::string & stage_label) const
{
  const auto time_arg = Moose::currentState();

  const FaceInfo * target_top_face = nullptr;
  const FaceInfo * target_top_left_face = nullptr;
  const FaceInfo * target_left_upper_face = nullptr;
  const FaceInfo * worst_top_gradient_face = nullptr;
  const FaceInfo * worst_top_target_flux_face = nullptr;
  const FaceInfo * worst_top_backflow_face = nullptr;
  const FaceInfo * worst_top_outflow_face = nullptr;

  Real x_min = std::numeric_limits<Real>::max();
  Real x_max = -std::numeric_limits<Real>::max();
  Real y_min = std::numeric_limits<Real>::max();
  Real y_max = -std::numeric_limits<Real>::max();
  unsigned int top_backflow_faces = 0;
  unsigned int top_outflow_faces = 0;
  Real top_backflow_target_flux_sum = 0.0;
  Real top_outflow_target_flux_sum = 0.0;

  for (const auto * fi : _flow_face_info)
  {
    const auto centroid = fi->faceCentroid();
    x_min = std::min(x_min, centroid(0));
    x_max = std::max(x_max, centroid(0));
    y_min = std::min(y_min, centroid(1));
    y_max = std::max(y_max, centroid(1));
  }

  const Real x_mid = 0.5 * (x_min + x_max);
  const Real y_upper_band = y_min + 0.7 * (y_max - y_min);
  Real best_top_metric = std::numeric_limits<Real>::max();
  Real best_top_left_metric = std::numeric_limits<Real>::max();
  Real best_left_upper_metric = std::numeric_limits<Real>::max();
  Real worst_top_gradient = -1.0;
  Real worst_top_target_flux = -1.0;
  Real worst_top_backflow = -1.0;
  Real worst_top_outflow = -1.0;

  for (const auto * fi : _flow_face_info)
  {
    if (_vel[0]->isInternalFace(*fi))
      continue;

    const auto centroid = fi->faceCentroid();
    const auto normal = fi->normal();

    if (std::abs(normal(1)) >= 0.999)
    {
      const Real top_metric = std::abs(centroid(1) - y_max) + 0.1 * std::abs(centroid(0) - x_mid);
      if (top_metric < best_top_metric)
      {
        best_top_metric = top_metric;
        target_top_face = fi;
      }

      const Real top_left_metric = std::abs(centroid(1) - y_max) + std::abs(centroid(0) - x_min);
      if (top_left_metric < best_top_left_metric)
      {
        best_top_left_metric = top_left_metric;
        target_top_left_face = fi;
      }

      const Real top_gradient = _pressure_boundary_normal_gradient_valid
                                    ? std::abs(libmesh_map_find(_pressure_boundary_normal_gradient,
                                                                fi->id()))
                                    : 0.0;
      if (top_gradient > worst_top_gradient)
      {
        worst_top_gradient = top_gradient;
        worst_top_gradient_face = fi;
      }

      const Real top_target_flux = std::abs(boundaryMassFluxTarget(fi, time_arg));
      if (top_target_flux > worst_top_target_flux)
      {
        worst_top_target_flux = top_target_flux;
        worst_top_target_flux_face = fi;
      }

      const Real signed_top_target_flux = boundaryMassFluxTarget(fi, time_arg);
      if (signed_top_target_flux < 0.0)
      {
        top_backflow_faces++;
        top_backflow_target_flux_sum += signed_top_target_flux;
        if (std::abs(signed_top_target_flux) > worst_top_backflow)
        {
          worst_top_backflow = std::abs(signed_top_target_flux);
          worst_top_backflow_face = fi;
        }
      }
      else if (signed_top_target_flux > 0.0)
      {
        top_outflow_faces++;
        top_outflow_target_flux_sum += signed_top_target_flux;
        if (signed_top_target_flux > worst_top_outflow)
        {
          worst_top_outflow = signed_top_target_flux;
          worst_top_outflow_face = fi;
        }
      }
    }
    else if (std::abs(normal(0)) >= 0.999)
    {
      const Real left_upper_metric =
          std::abs(centroid(0) - x_min) + std::abs(centroid(1) - y_upper_band);
      if (left_upper_metric < best_left_upper_metric)
      {
        best_left_upper_metric = left_upper_metric;
        target_left_upper_face = fi;
      }
    }
  }

  auto audit_face = [this, &time_arg, &stage_label](const FaceInfo * face, const std::string & label)
  {
    if (!face || face->boundaryIDs().empty())
      return;

    mooseAssert(face->boundaryIDs().size() == 1,
                "Expected a single boundary id on a FV boundary face.");

    auto * bc_pointer = _p->getBoundaryCondition(*face->boundaryIDs().begin());
    if (!bc_pointer)
      return;

    bc_pointer->setupFaceData(
        face, face->faceType(std::make_pair(_p->number(), _global_pressure_system_number)));

    const bool elem_is_fluid = hasBlocks(face->elemPtr()->subdomain_id());
    const ElemInfo & fluid_elem_info = elem_is_fluid ? *face->elemInfo() : *face->neighborInfo();
    const Elem * const boundary_elem = elem_is_fluid ? face->elemPtr() : face->neighborPtr();
    const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
    const Moose::FaceArg boundary_face{
        face, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};
    const Real p_elem = _p->getElemValue(fluid_elem_info, time_arg);
    const Real face_rho = _rho(boundary_face, time_arg);
    const Real predictor_base_flux = libmesh_map_find(_pressure_predictor_base_flux, face->id());
    const Real predictor_adjustment = pressurePredictorFluxAdjustment(face);
    const Real phi_hbya = libmesh_map_find(_phiHbyA_flux, face->id());
    const Real target_flux = boundaryMassFluxTarget(face, time_arg);
    const Real required_pressure_flux = phi_hbya + target_flux;
    const Real normal_ainv = boundaryNormalAinv(face);
    const Real cached_sn_grad = _pressure_boundary_normal_gradient_valid
                                    ? libmesh_map_find(_pressure_boundary_normal_gradient,
                                                       face->id())
                                    : 0.0;
    const Real bc_value = bc_pointer->computeBoundaryValue();
    const Real bc_sn_grad = bc_pointer->computeBoundaryNormalGradient();
    const Real pressure_flux = _pressure_equation_flux_valid
                                   ? libmesh_map_find(_pressure_equation_flux, face->id())
                                   : 0.0;
    const Real stored_face_flux = libmesh_map_find(_face_mass_flux, face->id());

    RealVectorValue boundary_velocity;
    RealVectorValue cell_velocity;
    std::ostringstream velocity_bc_types;
    for (const auto component : index_range(_vel))
    {
      boundary_velocity(component) = boundaryVelocityValue(face, component, time_arg);
      cell_velocity(component) = _vel[component]->getElemValue(fluid_elem_info, time_arg);

      if (component)
        velocity_bc_types << ",";

      if (!face->boundaryIDs().empty())
      {
        if (auto * vel_bc_pointer = _vel[component]->getBoundaryCondition(*face->boundaryIDs().begin()))
          velocity_bc_types << vel_bc_pointer->type();
        else
          velocity_bc_types << "none";
      }
      else
        velocity_bc_types << "none";
    }

    const Real boundary_normal_velocity =
        boundary_normal_multiplier * (boundary_velocity * face->normal());
    const Real cell_normal_velocity = boundary_normal_multiplier * (cell_velocity * face->normal());

    _console << "Pressure-boundary update audit (" << stage_label << ", " << label
             << "): face_id=" << face->id() << ", centroid=" << face->faceCentroid()
             << ", normal=" << face->normal() << ", p_elem=" << p_elem
             << ", pressure_bc_type=" << bc_pointer->type()
             << ", predictor_base_flux=" << predictor_base_flux
             << ", predictor_adjustment=" << predictor_adjustment
             << ", phiHbyA=" << phi_hbya << ", target_flux=" << target_flux
             << ", required_pressure_flux=" << required_pressure_flux
             << ", normal_ainv=" << normal_ainv
             << ", cached_sn_grad_p=" << cached_sn_grad
             << ", bc_value=" << bc_value
             << ", bc_sn_grad_p=" << bc_sn_grad
             << ", pressure_equation_flux=" << pressure_flux
             << ", stored_face_flux=" << stored_face_flux
             << ", face_rho=" << face_rho
             << ", boundary_velocity=" << boundary_velocity
             << ", cell_velocity=" << cell_velocity
             << ", boundary_normal_velocity=" << boundary_normal_velocity
             << ", cell_normal_velocity=" << cell_normal_velocity
             << ", velocity_bc_types=" << velocity_bc_types.str()
             << ", adjustable=" << isAdjustablePressureBoundaryFace(face) << std::endl;
  };

  audit_face(target_top_face, "top");
  audit_face(target_top_left_face, "top_left");
  audit_face(target_left_upper_face, "left_upper");
  audit_face(worst_top_gradient_face, "top_worst_gradient");
  audit_face(worst_top_target_flux_face, "top_worst_target_flux");
  audit_face(worst_top_backflow_face, "top_worst_backflow");
  audit_face(worst_top_outflow_face, "top_worst_outflow");

  _console << "Pressure-boundary top-flow summary (" << stage_label
           << "): top_backflow_faces=" << top_backflow_faces
           << ", top_outflow_faces=" << top_outflow_faces
           << ", top_backflow_target_flux_sum=" << top_backflow_target_flux_sum
           << ", top_outflow_target_flux_sum=" << top_outflow_target_flux_sum
           << ", worst_top_backflow=" << std::max(0.0, worst_top_backflow)
           << ", worst_top_outflow=" << std::max(0.0, worst_top_outflow) << std::endl;
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
    _phiHbyA_flux[fi->id()];
    _phig_flux[fi->id()];
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

  // We loop through the faces and populate the coupling fields (face H/A and 1/A)
  for (auto & fi : _flow_face_info)
  {
    Real face_rho = 0;
    RealVectorValue face_hbya;
    RealVectorValue density_times_face_hbya;

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
        interpolate(Moose::FV::InterpMethod::Average,
                    density_times_face_hbya(dim_i),
                    elem_rho * hbya_reader[dim_i](elem_dof),
                    neighbor_rho * hbya_reader[dim_i](neighbor_dof),
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
      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};

      const auto boundary_value_from_bc = [this, fi](const unsigned int dim_i)
      {
        if (fi->boundaryIDs().empty())
          return std::numeric_limits<Real>::quiet_NaN();

        mooseAssert(fi->boundaryIDs().size() == 1,
                    "Expected at most one physical boundary id on a FV boundary face.");
        if (auto * bc_pointer = _vel[dim_i]->getBoundaryCondition(*fi->boundaryIDs().begin()))
        {
          bc_pointer->setupFaceData(
              fi,
              fi->faceType(
                  std::make_pair(_vel[dim_i]->number(), _vel[dim_i]->sys().number())));
          return bc_pointer->computeBoundaryValue();
        }

        return std::numeric_limits<Real>::quiet_NaN();
      };

      bool use_constrained_boundary_state = _vel[0]->isDirichletBoundaryFace(*fi);
      if (!use_constrained_boundary_state && !fi->boundaryIDs().empty())
        for (const auto dim_i : make_range(_dim))
          if (auto * bc_pointer = _vel[dim_i]->getBoundaryCondition(*fi->boundaryIDs().begin()))
          {
            if (auto * pressure_inlet_outlet_bc =
                    dynamic_cast<LinearFVPressureInletOutletVelocityBC *>(bc_pointer))
            {
              pressure_inlet_outlet_bc->setupFaceData(
                  fi,
                  fi->faceType(
                      std::make_pair(_vel[dim_i]->number(), _vel[dim_i]->sys().number())));
              if (pressure_inlet_outlet_bc->computeBoundaryGradientMatrixContribution() > 0.0)
              {
                use_constrained_boundary_state = true;
                break;
              }
            }
          }

      // Local constrainHbyA analogue: only use the live boundary value when the
      // velocity patch is actually fixed on this iteration (Dirichlet or inletOutlet
      // backflow). Pure outflow faces still use the predictor cell state so the
      // pressure solve can supply the required outlet correction.
      if (use_constrained_boundary_state)
      {
        face_rho = _rho(boundary_face, Moose::currentState());
        for (const auto dim_i : make_range(_dim))
        {
          const Real boundary_value = boundary_value_from_bc(dim_i);
          face_hbya(dim_i) =
              std::isfinite(boundary_value)
                  ? -boundary_value
                  : -MetaPhysicL::raw_value((*_vel[dim_i])(boundary_face, Moose::currentState()));

          if (!_split_momentum_predictor_operator && !_body_force_kernel_names.empty())
            for (const auto & force_kernel : _body_force_kernels[dim_i])
            {
              force_kernel->setCurrentElemInfo(&elem_info);
              face_hbya(dim_i) -=
                  force_kernel->computeRightHandSideContribution() * ainv_reader[dim_i](elem_dof) /
                  (elem_info.volume() * elem_info.coordFactor()); // zero-term expansion
            }
          face_hbya(dim_i) *= boundary_normal_multiplier;
          density_times_face_hbya(dim_i) = face_rho * face_hbya(dim_i);
        }
      }
      // Otherwise we just do a one-term expansion (so we just use the element value)
      else
      {
        face_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
        for (const auto dim_i : make_range(_dim))
        {
          face_hbya(dim_i) = boundary_normal_multiplier * hbya_reader[dim_i](elem_dof);
          density_times_face_hbya(dim_i) = face_rho * face_hbya(dim_i);
        }
      }

      // We just do a one-term expansion for 1/A no matter what
      const Real elem_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
      for (const auto dim_i : index_range(raw_Ainv))
        Ainv(dim_i) = elem_rho * ainv_reader[dim_i](elem_dof);
    }
    // Lastly, we populate the face flux resulted by H/A
    _HbyA_flux[fi->id()] = density_times_face_hbya * fi->normal();
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

  _pressure_equation_flux_valid = false;
  _pressure_boundary_normal_gradient_valid = false;
  _pressure_predictor_face_state_valid = false;

  auto & pressure_gradient = selectPressureGradient(with_updated_pressure);
  const bool use_cached_predictor_operator = canUseCachedMomentumPredictorOperator();
  const bool split_predictor_operator = _split_momentum_predictor_operator;

  if (verbose && use_cached_predictor_operator)
    _console << "Using cached assembled momentum predictor operator for HbyA/Ainv." << std::endl;

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

    if (use_cached_predictor_operator)
    {
      _Ainv_raw.push_back(_cached_predictor_diagonal_raw[system_i]->clone());
      _HbyA_raw.push_back(_cached_predictor_operator_base_raw[system_i]->clone());
    }
    else
    {
      _Ainv_raw.push_back(current_local_solution.zero_clone());
      _HbyA_raw.push_back(current_local_solution.zero_clone());
      computePredictorOperatorBase(system_i, *(_HbyA_raw.back()), *(_Ainv_raw.back()));
    }

    NumericVector<Number> & Ainv = *(_Ainv_raw.back());
    NumericVector<Number> & HbyA = *(_HbyA_raw.back());

    // We create a working vector to ease some of the operations.
    auto working_vector = momentum_system->current_local_solution->zero_clone();
    PetscVector<Number> * working_vector_petsc =
        dynamic_cast<PetscVector<Number> *>(working_vector.get());
    mooseAssert(working_vector_petsc,
                "The vectors used in the RhieChowMassFlux objects need to be convertable "
                "to PetscVectors!");

    if (!split_predictor_operator)
    {
      // The legacy predictor operator includes explicit pressure/body-force
      // terms in the assembled RHS, so HbyA has to remove pressure and then
      // restore explicit body-force sources.
      working_vector_petsc->pointwise_mult(*pressure_gradient[system_i], *_cell_volumes);
      HbyA.add(-1.0, *working_vector_petsc);

      if (!_body_force_kernels.empty())
      {
        auto explicit_body_force_rhs = current_local_solution.zero_clone();
        for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
          if (hasBlocks(elem_info->subdomain_id()))
          {
            const auto elem_dof =
                elem_info->dofIndices()[_global_momentum_system_numbers[system_i]][0];

            Real rhs_contribution = 0.0;
            for (const auto & force_kernel : _body_force_kernels[system_i])
            {
              force_kernel->setCurrentElemInfo(elem_info);
              rhs_contribution += force_kernel->computeRightHandSideContribution();
            }

            explicit_body_force_rhs->set(elem_dof, rhs_contribution);
          }

        explicit_body_force_rhs->close();
        HbyA.add(1.0, *explicit_body_force_rhs);
      }
    }

    if (verbose)
    {
      _console << "total RHS" << std::endl;
      rhs.print();
      if (!split_predictor_operator)
      {
        _console << "pressure RHS" << std::endl;
        pressure_gradient[system_i]->print();
        _console << " predictor_base - pressure + explicit_body_force" << std::endl;
      }
      else
        _console << " predictor_base (split operator)" << std::endl;
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
      working_vector_petsc->pointwise_mult(*Ainv_full, *pressure_gradient[system_i]);
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
  if (updated_pressure || _grad_p_current.empty())
  {
    _grad_p_current.clear();
    for (const auto & component : _pressure_system->linearFVGradientContainer())
      _grad_p_current.push_back(component->clone());
  }

  return _grad_p_current;
}

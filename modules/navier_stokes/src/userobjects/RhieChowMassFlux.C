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
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "LinearSystem.h"
#include "LinearFVBoundaryCondition.h"

// libMesh includes
#include "libmesh/mesh_base.h"
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
  params.addRequiredParam<std::string>("p_diffusion_kernel",
                                       "The diffusion kernel acting on the pressure.");
  params.addParam<std::vector<std::vector<std::string>>>(
      "body_force_kernel_names",
      {},
      "The body force kernel names."
      "this double vector would have size index_x_dim: 'f1x f2x; f1y f2y; f1z f2z'");

  params.addRequiredParam<MooseFunctorName>(NS::density, "Density functor");
  params.addParam<MooseFunctorName>(
      NS::porosity, "1", "Porosity functor (defaults to 1 for non-porous calculations).");

  params.addParam<std::vector<BoundaryName>>(
      "pressure_baffle_sidesets", {}, "Sidesets which define porous pressure baffles.");
  params.addRangeCheckedParam<Real>("pressure_baffle_relaxation",
                                    1.0,
                                    "0.0<pressure_baffle_relaxation<=1.0",
                                    "Under-relaxation factor for pressure baffle jump updates.");
  params.addParam<bool>("debug_baffle", false, "Enable debug output for baffle jumps.");
  params.addParam<bool>("use_flux_velocity_reconstruction",
                        false,
                        "Reconstruct cell velocity from corrected face fluxes "
                        "using an oscillation-free least-squares fit.");
  params.addRangeCheckedParam<Real>(
      "flux_velocity_reconstruction_relaxation",
      1.0,
      "0.0<flux_velocity_reconstruction_relaxation<=1.0",
      "Under-relaxation factor for flux-based velocity reconstruction.");
  params.addParam<bool>("use_corrected_pressure_gradient",
                        true,
                        "Whether to use the baffle-corrected pressure gradient when forming HbyA.");
  params.addParam<std::vector<BoundaryName>>(
      "pressure_gradient_limiter",
      {},
      "Sidesets on which the pressure gradient uses a one-term expansion.");

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
    _face_velocity(_moose_mesh, blockIDs(), "face_velocity"),
    _face_mass_flux(
        declareRestartableData<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>>(
            "face_flux", _moose_mesh, blockIDs(), "face_values")),
    _body_force_kernel_names(
        getParam<std::vector<std::vector<std::string>>>("body_force_kernel_names")),
    _rho(getFunctor<Real>(NS::density)),
    _eps(getFunctor<Real>(NS::porosity)),
    _pressure_baffle_relaxation(getParam<Real>("pressure_baffle_relaxation")),
    _debug_baffle(getParam<bool>("debug_baffle")),
    _use_flux_velocity_reconstruction(getParam<bool>("use_flux_velocity_reconstruction")),
    _flux_velocity_reconstruction_relaxation(
        getParam<Real>("flux_velocity_reconstruction_relaxation")),
    _use_corrected_pressure_gradient(getParam<bool>("use_corrected_pressure_gradient")),
    _use_harmonic_Ainv_interp(isParamSetByUser(NS::porosity)),
    _baffle_jump(
        declareRestartableData<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>>(
            "baffle_jump", _moose_mesh, blockIDs(), "baffle_jump")),
    _pressure_projection_method(getParam<MooseEnum>("pressure_projection_method"))
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

  // Cache baffle and limiter boundary IDs
  const auto & baffle_names = getParam<std::vector<BoundaryName>>("pressure_baffle_sidesets");
  const auto & limiter_names = getParam<std::vector<BoundaryName>>("pressure_gradient_limiter");
  const auto baffle_ids = _moose_mesh.getBoundaryIDs(baffle_names);
  _pressure_baffle_boundary_ids.insert(baffle_ids.begin(), baffle_ids.end());
  const auto limiter_ids = _moose_mesh.getBoundaryIDs(limiter_names);
  _pressure_gradient_limiter_ids.insert(limiter_ids.begin(), limiter_ids.end());

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
  _Ainv.clear();
  _face_velocity.clear();
  _face_mass_flux.clear();
  _baffle_jump.clear();
  _grad_p_corrected.clear();
  _grad_p_current.clear();
  _grad_w_prev.clear();
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
  _p_diffusion_kernel = dynamic_cast<LinearFVAnisotropicDiffusion *>(flux_kernel[0]);
  if (!_p_diffusion_kernel)
    paramError("p_diffusion_kernel",
               "The provided diffusion kernel should of type LinearFVAnisotropicDiffusion!");

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
  _cell_porosity = _pressure_system->currentSolution()->zero_clone();
  const auto time_arg = Moose::currentState();
  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
    // We have to check this because the variable might not be defined on the given
    // block
    if (hasBlocks(elem_info->subdomain_id()))
    {
      const auto elem_dof = elem_info->dofIndices()[_global_pressure_system_number][0];
      _cell_volumes->set(elem_dof, elem_info->volume() * elem_info->coordFactor());
      _cell_porosity->set(elem_dof, _eps(makeElemArg(elem_info->elem()), time_arg));
    }

  _cell_volumes->close();
  _cell_porosity->close();

  _flow_face_info.clear();
  for (auto & fi : _fe_problem.mesh().faceInfo())
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
      _flow_face_info.push_back(fi);
}

void
RhieChowMassFlux::initialize()
{
  for (const auto & pair : _HbyA_flux)
    _HbyA_flux[pair.first] = 0;

  for (const auto & pair : _Ainv)
    _Ainv[pair.first] = 0;

  for (const auto & pair : _face_velocity)
    _face_velocity[pair.first] = RealVectorValue();

  for (const auto & pair : _baffle_jump)
    _baffle_jump[pair.first] = 0;

  for (auto & comp_vec : _grad_w_prev)
    for (auto & grad_vec : comp_vec)
      if (grad_vec)
        grad_vec->zero();
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
    // Ensure baffle storage exists for every face we might query later
    _baffle_jump[fi->id()] = 0.0;

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

  updateFaceVelocityFromMassFlux();
  computeCorrectedPressureGradient();
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
      const auto neighbor_rhs_contribution =
          _p_diffusion_kernel->computeNeighborRightHandSideContribution();

      // Compute the face flux from the matrix and right hand side contributions
      p_grad_flux = (p_neighbor_value * neighbor_matrix_contribution +
                     p_elem_value * elem_matrix_contribution) -
                    elem_rhs_contribution;

      if (_debug_baffle && isBaffleFace(*fi))
      {
        const Real J = _baffle_jump.evaluate(fi);
        const Real jump_elem = getSignedBaffleJump(*fi, /*elem_side=*/true);
        const Real jump_neigh = getSignedBaffleJump(*fi, /*elem_side=*/false);
        _console << "Baffle flux face " << fi->id() << " p_elem=" << p_elem_value
                 << " p_neigh=" << p_neighbor_value << " elem_mat=" << elem_matrix_contribution
                 << " neigh_mat=" << neighbor_matrix_contribution
                 << " elem_rhs=" << elem_rhs_contribution
                 << " neigh_rhs=" << neighbor_rhs_contribution << " J=" << J
                 << " jump_elem=" << jump_elem << " jump_neigh=" << jump_neigh
                 << " p_grad_flux=" << p_grad_flux << std::endl;
      }
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
    const Real face_flux = -_HbyA_flux[fi->id()] + p_grad_flux;
    _face_mass_flux[fi->id()] = face_flux;

    if (_debug_baffle && isBaffleFace(*fi))
      _console << "Baffle face mass flux " << fi->id() << " HbyA_flux=" << _HbyA_flux[fi->id()]
               << " p_grad_flux=" << p_grad_flux << " face_mass_flux=" << face_flux << std::endl;
  }

  updateFaceVelocityFromMassFlux();
  computeCorrectedPressureGradient();
}

void
RhieChowMassFlux::updateFaceVelocityFromMassFlux()
{
  const auto time_arg = Moose::currentState();
  for (auto & fi : _flow_face_info)
  {
    Real face_rho = 0.0;
    if (_vel[0]->isInternalFace(*fi))
    {
      const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
      const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);
      Moose::FV::interpolate(
          Moose::FV::InterpMethod::Average, face_rho, elem_rho, neighbor_rho, *fi, true);
    }
    else
    {
      const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
      const Elem * const boundary_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};
      face_rho = _rho(boundary_face, time_arg);
    }

    const Real phi = _face_mass_flux.evaluate(fi);
    const Real u_n = face_rho != 0.0 ? phi / face_rho : 0.0;

    const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
    const Point normal = elem_is_fluid ? fi->normal() : Point(-fi->normal());
    _face_velocity[fi->id()] = u_n * normal;
  }
}

void
RhieChowMassFlux::updateGradPrevFromFaceVelocity()
{
  if (_grad_w_prev.empty())
    return;

  for (auto & comp_vec : _grad_w_prev)
    for (auto & grad_vec : comp_vec)
      grad_vec->zero();

  std::vector<unsigned int> var_nums;
  for (const auto comp_index : make_range(_dim))
    var_nums.push_back(
        _momentum_implicit_systems[comp_index]->variable_number(_vel[comp_index]->name()));

  const auto & mesh = _fe_problem.mesh();
  const auto rz_radial_coord = mesh.getAxisymmetricRadialCoord();

  for (const auto & elem_info : mesh.elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const Elem & elem = *elem_info->elem();
    std::vector<RealVectorValue> sum(_dim, RealVectorValue());

    auto act = [&](const Elem &,
                   const Elem * const,
                   const FaceInfo * const fi,
                   const Point & surface_vector,
                   const Real /*coord*/,
                   const bool /*elem_has_info*/)
    {
      const RealVectorValue face_vel = _face_velocity.evaluate(fi);
      for (const auto comp_index : make_range(_dim))
        sum[comp_index] += surface_vector * face_vel(comp_index);
    };

    Moose::FV::loopOverElemFaceInfo(
        elem, _moose_mesh, act, mesh.getCoordSystem(elem.subdomain_id()), rz_radial_coord);

    const Real volume = elem_info->volume() * elem_info->coordFactor();
    const Real inv_volume = volume != 0.0 ? 1.0 / volume : 0.0;

    for (const auto comp_index : make_range(_dim))
    {
      const unsigned int system_number = _momentum_implicit_systems[comp_index]->number();
      const auto dof = elem.dof_number(system_number, var_nums[comp_index], 0);
      for (const auto dir_index : make_range(_dim))
        _grad_w_prev[comp_index][dir_index]->set(dof, sum[comp_index](dir_index) * inv_volume);
    }
  }

  for (auto & comp_vec : _grad_w_prev)
    for (auto & grad_vec : comp_vec)
      grad_vec->close();
}

void
RhieChowMassFlux::computeCellVelocity()
{
  if (_use_flux_velocity_reconstruction)
  {
    updateFaceVelocityFromMassFlux();

    if (_grad_w_prev.empty())
    {
      _grad_w_prev.resize(_dim);
      for (const auto comp_index : make_range(_dim))
      {
        _grad_w_prev[comp_index].resize(_dim);
        for (const auto dir_index : make_range(_dim))
          _grad_w_prev[comp_index][dir_index] =
              _momentum_implicit_systems[comp_index]->current_local_solution->zero_clone();
      }
    }

    std::vector<unsigned int> var_nums;
    for (const auto comp_index : make_range(_dim))
      var_nums.push_back(
          _momentum_implicit_systems[comp_index]->variable_number(_vel[comp_index]->name()));

    const auto time_arg = Moose::currentState();
    const auto & mesh = _fe_problem.mesh();
    const auto rz_radial_coord = mesh.getAxisymmetricRadialCoord();

    for (const auto & elem_info : mesh.elemInfoVector())
    {
      if (!hasBlocks(elem_info->subdomain_id()))
        continue;

      DenseMatrix<Real> M(_dim, _dim);
      DenseVector<Real> b(_dim);
      M.zero();
      b.zero();

      const Elem & elem = *elem_info->elem();

      auto act = [&](const Elem &,
                     const Elem * const neighbor,
                     const FaceInfo * const fi,
                     const Point & surface_vector,
                     const Real /*coord*/,
                     const bool elem_has_info)
      {
        const Real magS = surface_vector.norm();
        if (magS == 0.0)
          return;

        const Elem * const elem_face = elem_has_info ? fi->elemPtr() : fi->neighborPtr();
        const Elem * const neighbor_face = elem_has_info ? fi->neighborPtr() : fi->elemPtr();

        const bool neighbor_active =
            neighbor && neighbor_face && hasBlocks(neighbor_face->subdomain_id());

        Real face_rho = 0.0;
        if (neighbor_active)
        {
          const Real elem_rho = _rho(makeElemArg(elem_face), time_arg);
          const Real neighbor_rho = _rho(makeElemArg(neighbor_face), time_arg);
          Moose::FV::interpolate(
              Moose::FV::InterpMethod::Average, face_rho, elem_rho, neighbor_rho, *fi, true);
        }
        else
        {
          const Moose::FaceArg boundary_face{
              fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_face, nullptr};
          face_rho = _rho(boundary_face, time_arg);
        }

        const Real phi_raw = _face_mass_flux.evaluate(fi);
        // Make phi consistent with the outward surface_vector for THIS elem
        const Real phi = elem_has_info ? phi_raw : -phi_raw;
        const Real F = face_rho != 0.0 ? phi / face_rho : 0.0;

        const Point dPf = fi->faceCentroid() - elem_info->centroid();

        std::vector<Real> grad_times_dpf(_dim, 0.0);
        if (!_grad_w_prev.empty())
        {
          for (const auto comp_index : make_range(_dim))
          {
            const unsigned int system_number = _momentum_implicit_systems[comp_index]->number();
            const unsigned int var_num = var_nums[comp_index];
            const auto elem_dof = elem_face->dof_number(system_number, var_num, 0);
            const auto neighbor_dof =
                neighbor_active ? neighbor_face->dof_number(system_number, var_num, 0) : elem_dof;

            Real comp_val = 0.0;
            for (const auto dir_index : make_range(_dim))
            {
              const Real grad_elem = (*_grad_w_prev[comp_index][dir_index])(elem_dof);
              const Real grad_neighbor = neighbor_active
                                             ? (*_grad_w_prev[comp_index][dir_index])(neighbor_dof)
                                             : grad_elem;
              const Real grad_face =
                  neighbor_active ? 0.5 * (grad_elem + grad_neighbor) : grad_elem;
              comp_val += grad_face * dPf(dir_index);
            }
            grad_times_dpf[comp_index] = comp_val;
          }
        }

        Real corr = 0.0;
        for (const auto comp_index : make_range(_dim))
          corr += grad_times_dpf[comp_index] * surface_vector(comp_index);

        for (const auto i : make_range(_dim))
        {
          b(i) += ((F - corr) / magS) * surface_vector(i);
          for (const auto j : make_range(_dim))
            M(i, j) += surface_vector(i) * surface_vector(j) / magS;
        }
      };

      Moose::FV::loopOverElemFaceInfo(
          elem, _moose_mesh, act, mesh.getCoordSystem(elem.subdomain_id()), rz_radial_coord);

      DenseVector<Real> result(_dim);
      if (_dim == 1)
      {
        const Real denom = M(0, 0);
        result(0) = denom != 0.0 ? b(0) / denom : 0.0;
      }
      else
        M.cholesky_solve(b, result);

      for (const auto comp_index : make_range(_dim))
      {
        const unsigned int system_number = _momentum_implicit_systems[comp_index]->number();
        const auto index = elem.dof_number(system_number, var_nums[comp_index], 0);
        const Real old_val =
            (*_momentum_implicit_systems[comp_index]->current_local_solution)(index);
        const Real alpha = _flux_velocity_reconstruction_relaxation;
        const Real new_val = (1.0 - alpha) * old_val + alpha * result(comp_index);
        _momentum_implicit_systems[comp_index]->solution->set(index, new_val);
      }
    }

    for (const auto system_i : index_range(_momentum_implicit_systems))
    {
      _momentum_implicit_systems[system_i]->solution->close();
      _momentum_implicit_systems[system_i]->update();
      _momentum_systems[system_i]->setSolution(
          *_momentum_implicit_systems[system_i]->current_local_solution);
    }

    updateGradPrevFromFaceVelocity();
    return;
  }

  auto & pressure_gradient =
      _grad_p_corrected.empty() ? _pressure_system->gradientContainer() : _grad_p_corrected;

  // We set the dof value in the solution vector the same logic applies:
  // u_C = -(H/A)_C - (1/A)_C*grad(p)_C where C is the cell index
  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    auto working_vector = _Ainv_raw[system_i]->clone();
    working_vector->pointwise_mult(*working_vector, *pressure_gradient[system_i]);
    if (_cell_porosity)
      working_vector->pointwise_mult(*working_vector, *_cell_porosity);
    working_vector->add(*_HbyA_raw[system_i]);
    working_vector->scale(-1.0);
    (*_momentum_implicit_systems[system_i]->solution) = *working_vector;
    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }

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
    _face_velocity[fi->id()];
    _baffle_jump[fi->id()];
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
      const Real elem_eps = _eps(makeElemArg(fi->elemPtr()), time_arg);
      const Real neighbor_eps = _eps(makeElemArg(fi->neighborPtr()), time_arg);

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
        if (_use_harmonic_Ainv_interp)
          Ainv(dim_i) = harmonicInterpolation(elem_rho * ainv_reader[dim_i](elem_dof),
                                              neighbor_rho * ainv_reader[dim_i](neighbor_dof),
                                              *fi,
                                              true);
        else
          interpolate(InterpMethod::Average,
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

      // If it is a Dirichlet BC, we use the dirichlet value the make sure the face flux
      // is consistent
      if (_vel[0]->isDirichletBoundaryFace(*fi))
      {
        const Moose::FaceArg boundary_face{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};
        face_rho = _rho(boundary_face, Moose::currentState());

        for (const auto dim_i : make_range(_dim))
        {

          face_hbya(dim_i) =
              -MetaPhysicL::raw_value((*_vel[dim_i])(boundary_face, Moose::currentState()));

          if (!_body_force_kernel_names.empty())
            for (const auto & force_kernel : _body_force_kernels[dim_i])
            {
              force_kernel->setCurrentElemInfo(&elem_info);
              face_hbya(dim_i) -=
                  force_kernel->computeRightHandSideContribution() * ainv_reader[dim_i](elem_dof) /
                  (elem_info.volume() * elem_info.coordFactor()); // zero-term expansion
            }
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

  updateBaffleJumps();

  auto & pressure_gradient = selectPressureGradient(with_updated_pressure);

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
    // so we have to correct them back
    working_vector_petsc->pointwise_mult(*pressure_gradient[system_i], *_cell_volumes);
    if (_cell_porosity)
      working_vector_petsc->pointwise_mult(*working_vector_petsc, *_cell_porosity);
    HbyA.add(-1.0, *working_vector_petsc);

    if (verbose)
    {
      _console << "total RHS" << std::endl;
      rhs.print();
      _console << "pressure RHS" << std::endl;
      pressure_gradient[system_i]->print();
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
      working_vector_petsc->pointwise_mult(*Ainv_full, *pressure_gradient[system_i]);
      working_vector_petsc->pointwise_mult(*working_vector_petsc, *_cell_volumes);
      HbyA.add(-1.0, *working_vector_petsc);

      // Correct Ainv
      Ainv = *Ainv_full_old;
    }

    if (_cell_porosity)
      Ainv.pointwise_mult(Ainv, *_cell_porosity);

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
  if (updated_pressure)
  {
    _grad_p_current.clear();
    const bool use_corrected = _use_corrected_pressure_gradient && !_grad_p_corrected.empty();
    const auto & source = use_corrected ? _grad_p_corrected : _pressure_system->gradientContainer();
    for (const auto & component : source)
      _grad_p_current.push_back(component->clone());
  }

  return _grad_p_current;
}

std::pair<Real, Real>
RhieChowMassFlux::getAdvectedInterpolationCoeffs(const FaceInfo & fi,
                                                 Moose::FV::InterpMethod method,
                                                 Real face_mass_flux,
                                                 bool apply_porosity_scaling) const
{
  auto coeffs = Moose::FV::interpCoeffs(method, fi, /*one_is_elem=*/true, face_mass_flux);

  const auto time_arg = Moose::currentState();

  if (apply_porosity_scaling && fi.neighborPtr() && _eps.hasBlocks(fi.elem().subdomain_id()) &&
      _eps.hasBlocks(fi.neighborPtr()->subdomain_id()))
  {
    const Real eps_elem = _eps(makeElemArg(fi.elemPtr()), time_arg);
    const Real eps_neighbor = _eps(makeElemArg(fi.neighborPtr()), time_arg);
    coeffs.first /= eps_elem;
    coeffs.second /= eps_neighbor;
  }
  else if (apply_porosity_scaling)
  {
    const Real eps_elem = _eps(makeElemArg(fi.elemPtr()), time_arg);
    coeffs.first /= eps_elem;
    coeffs.second = 0.0;
  }

  return coeffs;
}

Real
RhieChowMassFlux::getFaceSidePorosity(const FaceInfo & fi,
                                      bool elem_side,
                                      const Moose::StateArg & time) const
{
  const Elem * const elem = elem_side ? fi.elemPtr() : fi.neighborPtr();
  if (!elem)
    return 1.0;

  return _eps(makeElemArg(elem), time);
}

Real
RhieChowMassFlux::getSignedBaffleJump(const FaceInfo & fi, bool elem_side) const
{
  if (!isBaffleFace(fi))
    return 0.0;

  const Real J = _baffle_jump.evaluate(&fi);
  const bool elem_owner = elemIsBaffleOwner(fi);
  const bool side_is_owner = elem_side ? elem_owner : !elem_owner;
  return side_is_owner ? -J : J;
}

Real
RhieChowMassFlux::pressureGradient(const ElemInfo & elem_info, unsigned int component) const
{
  if (component >= _dim)
    return 0.0;

  const auto dof = elem_info.dofIndices()[_global_pressure_system_number][0];
  Real grad = 0.0;
  if (!_grad_p_corrected.empty())
    grad = (*_grad_p_corrected[component])(dof);
  else
  {
    const auto & grads = _pressure_system->gradientContainer();
    grad = (*grads[component])(dof);
  }

  return grad;

}

Real
RhieChowMassFlux::rawPressureGradient(const ElemInfo & elem_info, unsigned int component) const
{
  if (component >= _dim)
    return 0.0;

  const auto dof = elem_info.dofIndices()[_global_pressure_system_number][0];
  const auto & grads = _pressure_system->gradientContainer();
  Real grad = (*grads[component])(dof);
  return grad;
}

void
RhieChowMassFlux::updateBaffleJumps()
{
  if (_pressure_baffle_boundary_ids.empty())
    return;

  const auto time_arg = Moose::currentState();
  auto stringify_bnds = [](const std::vector<BoundaryID> & ids)
  {
    std::ostringstream os;
    os << "{";
    bool first = true;
    for (const auto id : ids)
    {
      if (!first)
        os << ",";
      os << id;
      first = false;
    }
    os << "}";
    return os.str();
  };

  for (auto & fi : _flow_face_info)
  {
    if (!isBaffleFace(*fi) || !fi->neighborPtr())
      continue;
    if (!hasBlocks(fi->elemPtr()->subdomain_id()) || !hasBlocks(fi->neighborPtr()->subdomain_id()))
      continue;

    const bool elem_owner = elemIsBaffleOwner(*fi);
    const Elem * const owner_elem = elem_owner ? fi->elemPtr() : fi->neighborPtr();
    const Elem * const neighbor_elem = elem_owner ? fi->neighborPtr() : fi->elemPtr();

    const auto owner_rho = _rho(makeElemArg(owner_elem), time_arg);
    const auto neighbor_rho = _rho(makeElemArg(neighbor_elem), time_arg);

    Real face_rho = 0.0;
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, face_rho, owner_rho, neighbor_rho, *fi, true);

    const Real phi = _face_mass_flux.evaluate(fi);
    const Real U_n = face_rho != 0.0 ? phi / face_rho : 0.0;

    const Real eps_owner = _eps(makeElemArg(owner_elem), time_arg);
    const Real eps_neighbor = _eps(makeElemArg(neighbor_elem), time_arg);

    const Real u_owner = U_n / eps_owner;
    const Real u_neighbor = U_n / eps_neighbor;

    const Real J_new =
        0.5 * (owner_rho * u_owner * u_owner - neighbor_rho * u_neighbor * u_neighbor);
    const Real J_old = _baffle_jump[fi->id()];

    _baffle_jump[fi->id()] =
        _pressure_baffle_relaxation * J_new + (1.0 - _pressure_baffle_relaxation) * J_old;

    if (_debug_baffle)
    {
      const auto elem_bnds = _moose_mesh.getBoundaryIDs(fi->elemPtr(), fi->elemSideID());
      const auto neigh_bnds = _moose_mesh.getBoundaryIDs(fi->neighborPtr(), fi->neighborSideID());
      _console << "Baffle jump face " << fi->id() << " owner_is_elem=" << elem_owner
               << " elem_block=" << fi->elem().subdomain_id()
               << " neigh_block=" << fi->neighbor().subdomain_id()
               << " elem_bnds=" << stringify_bnds(elem_bnds)
               << " neigh_bnds=" << stringify_bnds(neigh_bnds) << " phi=" << phi << " U_n=" << U_n
               << " rho_f=" << face_rho << " eps_owner=" << eps_owner
               << " eps_neigh=" << eps_neighbor << " J_new=" << J_new
               << " J_old=" << J_old << " J=" << _baffle_jump[fi->id()]
               << std::endl;
    }
  }
}

void
RhieChowMassFlux::computeCorrectedPressureGradient()
{
  if (!_pressure_system)
    return;

  if (_pressure_baffle_boundary_ids.empty() && _pressure_gradient_limiter_ids.empty())
  {
    _grad_p_corrected.clear();
    return;
  }

  const auto time_arg = Moose::currentState();

  if (_grad_p_corrected.empty())
  {
    _grad_p_corrected.resize(_dim);
    for (const auto i : make_range(_dim))
      _grad_p_corrected[i] = _pressure_system->currentSolution()->zero_clone();
  }

  for (const auto i : make_range(_dim))
    _grad_p_corrected[i]->zero();

  PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);

  const auto & mesh = _fe_problem.mesh();
  const auto rz_radial_coord = mesh.getAxisymmetricRadialCoord();

  for (const auto & elem_info : mesh.elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const Elem & elem = *elem_info->elem();
    const auto coord_type = mesh.getCoordSystem(elem.subdomain_id());
    const auto elem_dof = elem_info->dofIndices()[_global_pressure_system_number][0];

    RealVectorValue sum(0.0);

    auto act = [&](const Elem &,
                   const Elem * const neighbor,
                   const FaceInfo * const fi,
                   const Point & surface_vector,
                   const Real /*coord*/,
                   const bool elem_has_info)
    {
      const Elem * const sided_elem = elem_has_info ? fi->elemPtr() : fi->neighborPtr();
      const ElemInfo * const elem_info_face = elem_has_info ? fi->elemInfo() : fi->neighborInfo();
      const ElemInfo * const neighbor_info_face =
          elem_has_info ? fi->neighborInfo() : fi->elemInfo();

      const auto elem_dof_face = elem_info_face->dofIndices()[_global_pressure_system_number][0];
      const Real p_elem = p_reader(elem_dof_face);

      Real p_face = p_elem;

      const bool neighbor_active =
          neighbor && neighbor_info_face && hasBlocks(neighbor_info_face->subdomain_id());

      if (neighbor_active && !isPressureGradientLimited(*fi))
      {
        const auto neighbor_dof =
            neighbor_info_face->dofIndices()[_global_pressure_system_number][0];
        const Real p_neighbor = p_reader(neighbor_dof);
        const auto coeffs =
            Moose::FV::interpCoeffs(Moose::FV::InterpMethod::Average, *fi, elem_has_info);
        const Real jump_side = getSignedBaffleJump(*fi, elem_has_info);
        p_face = coeffs.first * p_elem + coeffs.second * (p_neighbor + jump_side);
        if (_debug_baffle && isBaffleFace(*fi))
          _console << "Baffle grad face " << fi->id() << " elem_has_info=" << elem_has_info
                   << " p_elem=" << p_elem << " p_neigh=" << p_neighbor
                   << " jump_side=" << jump_side << " p_face=" << p_face << std::endl;
      }
      else if (!neighbor_active)
      {
        const Moose::FaceArg face_arg{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, sided_elem, nullptr};
        p_face = MetaPhysicL::raw_value((*_p)(face_arg, time_arg));
      }

      sum += surface_vector * p_face;
    };

    Moose::FV::loopOverElemFaceInfo(elem, _moose_mesh, act, coord_type, rz_radial_coord);

    const Real volume = (*_cell_volumes)(elem_dof);
    const Real inv_volume = volume != 0.0 ? 1.0 / volume : 0.0;

    for (const auto i : make_range(_dim))
      _grad_p_corrected[i]->set(elem_dof, sum(i) * inv_volume);
  }

  for (const auto i : make_range(_dim))
    _grad_p_corrected[i]->close();
}

bool
RhieChowMassFlux::isBaffleFace(const FaceInfo & fi) const
{
  if (_pressure_baffle_boundary_ids.empty())
    return false;

  for (const auto & bnd_id : fi.boundaryIDs())
    if (_pressure_baffle_boundary_ids.count(bnd_id))
      return true;

  return false;
}

bool
RhieChowMassFlux::elemIsBaffleOwner(const FaceInfo & fi) const
{
  if (!fi.neighborPtr())
    return true;

  bool elem_has_baffle = false;
  bool neighbor_has_baffle = false;

  const auto elem_bnd_ids = _moose_mesh.getBoundaryIDs(fi.elemPtr(), fi.elemSideID());
  for (const auto & bnd_id : elem_bnd_ids)
    if (_pressure_baffle_boundary_ids.count(bnd_id))
    {
      elem_has_baffle = true;
      break;
    }

  if (fi.neighborPtr())
  {
    const auto neighbor_bnd_ids = _moose_mesh.getBoundaryIDs(fi.neighborPtr(), fi.neighborSideID());
    for (const auto & bnd_id : neighbor_bnd_ids)
      if (_pressure_baffle_boundary_ids.count(bnd_id))
      {
        neighbor_has_baffle = true;
        break;
      }
  }

  if (elem_has_baffle != neighbor_has_baffle)
    return elem_has_baffle;

  return fi.elem().subdomain_id() <= fi.neighbor().subdomain_id();
}

bool
RhieChowMassFlux::isPressureGradientLimited(const FaceInfo & fi) const
{
  if (_pressure_gradient_limiter_ids.empty())
    return false;

  for (const auto & bnd_id : fi.boundaryIDs())
    if (_pressure_gradient_limiter_ids.count(bnd_id))
      return true;

  return false;
}

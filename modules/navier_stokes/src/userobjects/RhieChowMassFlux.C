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
#include "LinearFVMomentumPressure.h"
#include "LinearFVPressureFluxBC.h"
#include "FVReconstructedPressureGradient.h"
#include "FVUtils.h"

// libMesh includes
#include "libmesh/mesh_base.h"
#include "libmesh/elem.h"
#include "libmesh/elem_range.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

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
  params.addParam<std::string>(
      "momentum_pressure_kernel",
      "",
      "Momentum pressure kernel whose registered pressure gradient field should also be used by "
      "Rhie-Chow when constructing H/A.");
  params.addRangeCheckedParam<Real>(
      "reconstructed_pressure_gradient_feedback_relaxation",
      0.1,
      "0.0<reconstructed_pressure_gradient_feedback_relaxation<=1.0",
      "Relaxation factor applied when feeding reconstructed pressure gradients back into the "
      "momentum predictor.");
  params.addParam<MooseEnum>(
      "reconstructed_pressure_gradient_velocity_update",
      MooseEnum("reconstructed pressure_gradient", "reconstructed"),
      "How the velocity is updated when using FVReconstructedPressureGradient. 'reconstructed' "
      "writes the reconstructed velocity directly. 'pressure_gradient' updates the velocity from "
      "the published pressure gradient.");
  params.addParam<MooseEnum>(
      "reconstructed_pressure_gradient_boundary_cells",
      MooseEnum("reconstructed base_gradient", "reconstructed"),
      "Which pressure gradient is fed back on cells touching a boundary face.");
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
    _momentum_pressure_kernel_name(getParam<std::string>("momentum_pressure_kernel")),
    _reconstructed_pressure_gradient_ready(false),
    _rho(getFunctor<Real>(NS::density)),
    _pressure_system(nullptr),
    _pressure_gradient_field(nullptr),
    _global_pressure_system_number(0),
    _pressure_projection_method(getParam<MooseEnum>("pressure_projection_method")),
    _reconstructed_pressure_gradient_feedback_relaxation(
        getParam<Real>("reconstructed_pressure_gradient_feedback_relaxation")),
    _reconstructed_pressure_gradient_velocity_update(
        getParam<MooseEnum>("reconstructed_pressure_gradient_velocity_update")),
    _reconstructed_pressure_gradient_boundary_cells(
        getParam<MooseEnum>("reconstructed_pressure_gradient_boundary_cells"))
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

  if (_momentum_pressure_kernel_name.empty())
    _pressure_gradient_field = &pressure_var->computeCellGradients();
  else
  {
    std::vector<LinearFVElementalKernel *> kernels;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribThread>(_tid)
        .template condition<AttribSystem>("LinearFVElementalKernel")
        .template condition<AttribName>(_momentum_pressure_kernel_name)
        .queryInto(kernels);

    if (kernels.size() != 1)
      paramError("momentum_pressure_kernel",
                 "The kernel with the given name could not be found or multiple instances were "
                 "identified.");

    const auto * const pressure_kernel = dynamic_cast<const LinearFVMomentumPressure *>(kernels[0]);
    if (!pressure_kernel)
      paramError("momentum_pressure_kernel",
                 "The provided kernel should be of type LinearFVMomentumPressure.");

    if (dynamic_cast<const FVReconstructedPressureGradient *>(
            &pressure_kernel->pressureGradientField().method()) &&
        pressure_kernel->pressureVariableNumber() != _p->number())
      paramError("momentum_pressure_kernel",
                 "FVReconstructedPressureGradient can only be used for the pressure variable "
                 "registered on RhieChowMassFlux '",
                 name(),
                 "'.");

    _pressure_gradient_field = &pressure_kernel->pressureGradientField();
  }

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
}

void
RhieChowMassFlux::meshChanged()
{
  _HbyA_flux.clear();
  _Ainv.clear();
  _face_mass_flux.clear();
  _reconstructed_pressure_gradient.clear();
  _reconstruction_velocity_gradient.clear();
  _boundary_cell_ids.clear();
  _reconstructed_pressure_gradient_ready = false;
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
  _boundary_cell_ids.clear();
  for (auto & fi : _fe_problem.mesh().faceInfo())
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
    {
      _flow_face_info.push_back(fi);
      if (!fi->boundaryIDs().empty())
      {
        if (fi->elemPtr() && hasBlocks(fi->elemPtr()->subdomain_id()))
          _boundary_cell_ids.insert(fi->elemPtr()->id());
        if (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id()))
          _boundary_cell_ids.insert(fi->neighborPtr()->id());
      }
    }
}

void
RhieChowMassFlux::updateReconstructionVelocityGradient()
{
  if (_reconstruction_velocity_gradient.empty())
  {
    _reconstruction_velocity_gradient.resize(_dim);
    for (const auto component : make_range(_dim))
    {
      _reconstruction_velocity_gradient[component].resize(_dim);
      for (const auto direction : make_range(_dim))
        _reconstruction_velocity_gradient[component][direction] =
            _momentum_implicit_systems[component]->current_local_solution->zero_clone();
    }
  }

  for (auto & component_gradients : _reconstruction_velocity_gradient)
    for (auto & gradient : component_gradients)
      gradient->zero();

  const auto & mesh = _fe_problem.mesh();
  const auto rz_radial_coord = mesh.getAxisymmetricRadialCoord();
  const auto time_arg = Moose::currentState();

  for (const auto & elem_info : mesh.elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const Elem & elem = *elem_info->elem();
    std::vector<RealVectorValue> surface_sum(_dim, RealVectorValue());

    auto act = [&](const Elem &,
                   const Elem * const,
                   const FaceInfo * const fi,
                   const Point & surface_vector,
                   const Real,
                   const bool elem_has_info)
    {
      const ElemInfo * const neighbor_info = elem_has_info ? fi->neighborInfo() : fi->elemInfo();
      const bool neighbor_active = neighbor_info && hasBlocks(neighbor_info->subdomain_id());

      for (const auto component : make_range(_dim))
      {
        const Real elem_value = _vel[component]->getElemValue(*elem_info, time_arg);
        Real face_value = elem_value;

        if (neighbor_active)
        {
          const Real neighbor_value = _vel[component]->getElemValue(*neighbor_info, time_arg);
          Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                                 face_value,
                                 elem_value,
                                 neighbor_value,
                                 *fi,
                                 elem_has_info);
        }

        surface_sum[component] += surface_vector * face_value;
      }
    };

    Moose::FV::loopOverElemFaceInfo(
        elem, _moose_mesh, act, mesh.getCoordSystem(elem.subdomain_id()), rz_radial_coord);

    const Real volume = elem_info->volume() * elem_info->coordFactor();
    if (volume == 0.0)
      continue;

    for (const auto component : make_range(_dim))
    {
      const auto dof =
          elem_info
              ->dofIndices()[_global_momentum_system_numbers[component]][_vel[component]->number()];
      for (const auto direction : make_range(_dim))
        _reconstruction_velocity_gradient[component][direction]->set(
            dof, surface_sum[component](direction) / volume);
    }
  }

  // Aguerre's Taylor correction needs an accurate velocity gradient on skewed cells. Keep the
  // Green-Gauss-style result above as a fallback, and overwrite it with a cell-neighbor
  // least-squares gradient when the Cartesian stencil is usable.
  for (const auto & elem_info : mesh.elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    if (mesh.getCoordSystem(elem_info->subdomain_id()) != Moose::CoordinateSystemType::COORD_XYZ)
      continue;

    DenseMatrix<Real> least_squares_matrix(_dim, _dim);
    std::vector<DenseVector<Real>> least_squares_rhs(_dim, DenseVector<Real>(_dim));
    least_squares_matrix.zero();

    std::vector<Real> elem_values(_dim);
    for (const auto component : make_range(_dim))
      elem_values[component] = _vel[component]->getElemValue(*elem_info, time_arg);

    unsigned int neighbor_count = 0;
    const Elem & elem = *elem_info->elem();
    auto act = [&](const Elem &,
                   const Elem * const,
                   const FaceInfo * const fi,
                   const Point &,
                   const Real,
                   const bool elem_has_info)
    {
      const ElemInfo * const neighbor_info = elem_has_info ? fi->neighborInfo() : fi->elemInfo();
      if (!neighbor_info || !hasBlocks(neighbor_info->subdomain_id()))
        return;

      const Point d_cn = neighbor_info->centroid() - elem_info->centroid();
      if (d_cn.norm() == 0.0)
        return;

      neighbor_count++;
      for (const auto i : make_range(_dim))
      {
        for (const auto j : make_range(_dim))
          least_squares_matrix(i, j) += d_cn(i) * d_cn(j);

        for (const auto component : make_range(_dim))
          least_squares_rhs[component](i) +=
              (_vel[component]->getElemValue(*neighbor_info, time_arg) - elem_values[component]) *
              d_cn(i);
      }
    };

    Moose::FV::loopOverElemFaceInfo(
        elem, _moose_mesh, act, mesh.getCoordSystem(elem.subdomain_id()), rz_radial_coord);

    if (neighbor_count < _dim)
      continue;

    for (const auto component : make_range(_dim))
    {
      DenseVector<Real> gradient(_dim);
      DenseMatrix<Real> solve_matrix(least_squares_matrix);
      solve_matrix.svd_solve(least_squares_rhs[component], gradient);

      const auto dof =
          elem_info
              ->dofIndices()[_global_momentum_system_numbers[component]][_vel[component]->number()];
      for (const auto direction : make_range(_dim))
        _reconstruction_velocity_gradient[component][direction]->set(dof, gradient(direction));
    }
  }

  for (auto & component_gradients : _reconstruction_velocity_gradient)
    for (auto & gradient : component_gradients)
      gradient->close();
}

RealVectorValue
RhieChowMassFlux::reconstructionVelocityGradient(const ElemInfo & elem_info,
                                                 const FaceInfo & fi,
                                                 const bool elem_has_info,
                                                 const unsigned int velocity_component) const
{
  RealVectorValue elem_gradient;
  for (const auto direction : make_range(_dim))
    elem_gradient(direction) = (*_reconstruction_velocity_gradient[velocity_component][direction])(
        elem_info.dofIndices()[_global_momentum_system_numbers[velocity_component]]
                              [_vel[velocity_component]->number()]);

  const ElemInfo * const neighbor_info = elem_has_info ? fi.neighborInfo() : fi.elemInfo();
  if (!neighbor_info || !hasBlocks(neighbor_info->subdomain_id()))
    return elem_gradient;

  RealVectorValue neighbor_gradient;
  for (const auto direction : make_range(_dim))
    neighbor_gradient(direction) =
        (*_reconstruction_velocity_gradient[velocity_component][direction])(
            neighbor_info->dofIndices()[_global_momentum_system_numbers[velocity_component]]
                                       [_vel[velocity_component]->number()]);

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
RhieChowMassFlux::computeCellVelocityFromPressureGradient()
{
  const auto & pressure_gradient = pressureGradientComponents();

  // u_C = -(H/A)_C - (1/A)_C * grad(p)_C.
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
}

Real
RhieChowMassFlux::faceNormalValueFromFlux(const FaceInfo & fi,
                                          const Point & face_normal,
                                          const Real face_flux) const
{
  const auto time_arg = Moose::currentState();
  Real face_rho = 0.0;

  if (_vel[0]->isInternalFace(fi))
  {
    const Real elem_rho = _rho(makeElemArg(fi.elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi.neighborPtr()), time_arg);
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, face_rho, elem_rho, neighbor_rho, fi, true);
  }
  else
  {
    const bool elem_is_fluid = hasBlocks(fi.elemPtr()->subdomain_id());
    const Elem * const boundary_elem = elem_is_fluid ? fi.elemPtr() : fi.neighborPtr();
    const Moose::FaceArg boundary_face{
        &fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};
    face_rho = _rho(boundary_face, time_arg);
  }

  if (face_rho == 0.0)
    return 0.0;

  const Real normal_speed = face_flux / face_rho;
  const bool elem_is_fluid = hasBlocks(fi.elemPtr()->subdomain_id());
  const Point normal = elem_is_fluid ? fi.normal() : Point(-fi.normal());

  return normal_speed * (normal * face_normal);
}

Real
RhieChowMassFlux::faceNormalVelocityFromMassFlux(const FaceInfo & fi,
                                                 const Point & face_normal) const
{
  return faceNormalValueFromFlux(fi, face_normal, libmesh_map_find(_face_mass_flux, fi.id()));
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
RhieChowMassFlux::getHbyAFlux(const FaceInfo & fi) const
{
  return _HbyA_flux.evaluate(&fi);
}

RealVectorValue
RhieChowMassFlux::getFaceAinv(const FaceInfo & fi) const
{
  return _Ainv.evaluate(&fi);
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

void
RhieChowMassFlux::preparePressureGradientUpdate()
{
  _reconstructed_pressure_gradient_ready = false;
}

bool
RhieChowMassFlux::hasReconstructedPressureGradient() const
{
  return _reconstructed_pressure_gradient_ready && !_reconstructed_pressure_gradient.empty();
}

const std::vector<std::unique_ptr<NumericVector<Number>>> &
RhieChowMassFlux::reconstructedPressureGradientComponents() const
{
  mooseAssert(hasReconstructedPressureGradient(), "Reconstructed pressure gradient is not ready.");

  return _reconstructed_pressure_gradient;
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
}

void
RhieChowMassFlux::computeCellVelocity()
{
  if (usingReconstructedPressureGradientMethod())
  {
    const bool update_velocity_from_reconstruction =
        _reconstructed_pressure_gradient_velocity_update == "reconstructed";
    const bool use_base_gradient_on_boundary =
        _reconstructed_pressure_gradient_boundary_cells == "base_gradient";
    const auto & pressure_gradient = pressureGradientComponents();

    updateReconstructionVelocityGradient();

    if (_reconstructed_pressure_gradient.empty())
    {
      _reconstructed_pressure_gradient.resize(_dim);
      for (const auto component : make_range(_dim))
        _reconstructed_pressure_gradient[component] =
            _pressure_system->currentSolution()->zero_clone();
    }

    for (auto & pressure_gradient_vec : _reconstructed_pressure_gradient)
      pressure_gradient_vec->zero();

    const auto & mesh = _fe_problem.mesh();
    const auto rz_radial_coord = mesh.getAxisymmetricRadialCoord();

    for (const auto & elem_info : mesh.elemInfoVector())
    {
      if (!hasBlocks(elem_info->subdomain_id()))
        continue;

      const bool boundary_cell = _boundary_cell_ids.count(elem_info->elem()->id());
      DenseMatrix<Real> matrix(_dim, _dim);
      DenseVector<Real> projection_rhs(_dim);
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

        // Aguerre's formulation reconstructs the cell velocity from the total conservative face
        // flux after removing the Taylor face-flux contribution from the previous velocity
        // gradient.
        Real face_normal_reconstructed_quantity = faceNormalVelocityFromMassFlux(*fi, face_normal);

        const Point d_pf = fi->faceCentroid() - elem_info->centroid();
        Real gradient_flux_correction = 0.0;
        for (const auto component : make_range(_dim))
          gradient_flux_correction +=
              (reconstructionVelocityGradient(*elem_info, *fi, elem_has_info, component) * d_pf) *
              surface_vector(component);

        face_normal_reconstructed_quantity -= gradient_flux_correction / surface_area;

        for (const auto i : make_range(_dim))
        {
          projection_rhs(i) += face_normal_reconstructed_quantity * surface_vector(i);
          for (const auto j : make_range(_dim))
            matrix(i, j) += surface_vector(i) * surface_vector(j) / surface_area;
        }
      };

      Moose::FV::loopOverElemFaceInfo(
          elem, _moose_mesh, act, mesh.getCoordSystem(elem.subdomain_id()), rz_radial_coord);

      auto solve_projection = [&](const DenseVector<Real> & rhs, DenseVector<Real> & solution)
      {
        if (_dim == 1)
        {
          const Real denom = matrix(0, 0);
          solution(0) = denom != 0.0 ? rhs(0) / denom : 0.0;
        }
        else
        {
          DenseMatrix<Real> solve_matrix(matrix);
          solve_matrix.cholesky_solve(rhs, solution);
        }
      };

      DenseVector<Real> reconstructed_quantity(_dim);
      solve_projection(projection_rhs, reconstructed_quantity);

      // Always build the reconstructed pressure-gradient candidate. The velocity update can either
      // use the direct reconstructed value or the pressure-gradient field published below.
      for (const auto component : make_range(_dim))
      {
        const auto momentum_dof =
            elem_info->dofIndices()[_global_momentum_system_numbers[component]][0];
        const auto pressure_dof =
            elem_info->dofIndices()[_global_pressure_system_number][_p->number()];

        const Real HbyA = (*_HbyA_raw[component])(momentum_dof);
        const Real Ainv = (*_Ainv_raw[component])(momentum_dof);
        const Real base_gradient = (*pressure_gradient[component])(pressure_dof);
        const Real reconstructed_gradient =
            Ainv != 0.0 ? (-reconstructed_quantity(component) - HbyA) / Ainv : base_gradient;
        const Real direct_velocity = reconstructed_quantity(component);
        const Real stored_gradient =
            use_base_gradient_on_boundary && boundary_cell ? base_gradient : reconstructed_gradient;

        if (update_velocity_from_reconstruction)
          _momentum_implicit_systems[component]->solution->set(momentum_dof, direct_velocity);
        _reconstructed_pressure_gradient[component]->set(pressure_dof, stored_gradient);
      }
    }

    if (update_velocity_from_reconstruction)
    {
      for (const auto system_i : index_range(_momentum_implicit_systems))
      {
        _momentum_implicit_systems[system_i]->solution->close();
        _momentum_implicit_systems[system_i]->update();
        _momentum_systems[system_i]->setSolution(
            *_momentum_implicit_systems[system_i]->current_local_solution);
      }
    }

    for (auto & pressure_gradient_vec : _reconstructed_pressure_gradient)
      pressure_gradient_vec->close();

    _reconstructed_pressure_gradient_ready = true;
    _pressure_system->updateFVGradient(pressureGradientField());

    if (!update_velocity_from_reconstruction)
      computeCellVelocityFromPressureGradient();

    return;
  }

  computeCellVelocityFromPressureGradient();
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
    for (const auto & component : pressureGradientComponents())
      _grad_p_current.push_back(component->clone());
  }

  if (_grad_p_current.empty())
    for (const auto & component : pressureGradientComponents())
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

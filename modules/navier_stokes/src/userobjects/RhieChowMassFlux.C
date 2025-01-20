//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

  params.addRequiredParam<MooseFunctorName>(NS::density, "Density functor");

  // We disable the execution of this, should only provide functions
  // for the SIMPLE executioner
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_NONE);
  exec_enum = {EXEC_NONE};
  params.suppressParameter<ExecFlagEnum>("execute_on");

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
    _rho(getFunctor<Real>(NS::density))
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

  setupCellVolumes();
}

void
RhieChowMassFlux::meshChanged()
{
  _HbyA_flux.clear();
  _Ainv.clear();
  _face_mass_flux.clear();
  setupCellVolumes();
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
RhieChowMassFlux::setupCellVolumes()
{
  // We cache the cell volumes into a petsc vector for corrections here so we can use
  // the optimized petsc operations for the normalization
  _cell_volumes = _pressure_system->currentSolution()->zero_clone();
  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    const auto elem_dof = elem_info->dofIndices()[_global_pressure_system_number][0];
    _cell_volumes->set(elem_dof, elem_info->volume() * elem_info->coordFactor());
  }
  _cell_volumes->close();
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
  for (auto & fi : _fe_problem.mesh().faceInfo())
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
      const Elem * const boundary_elem =
          hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->elemPtr() : fi->neighborPtr();

      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};

      const Real face_rho = _rho(boundary_face, time_arg);
      for (const auto dim_i : index_range(_vel))
        density_times_velocity(dim_i) =
            face_rho * raw_value((*_vel[dim_i])(boundary_face, time_arg));
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

void
RhieChowMassFlux::computeFaceMassFlux()
{
  using namespace Moose::FV;

  const auto time_arg = Moose::currentState();

  // Petsc vector reader to make the repeated reading from the vector faster
  PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);

  // We loop through the faces and compute the face fluxes using the pressure gradient
  // and the momentum matrix/right hand side
  for (auto & fi : _fe_problem.mesh().faceInfo())
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
  auto & pressure_gradient = _pressure_system->gradientContainer();

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
  for (auto & fi : _fe_problem.mesh().faceInfo())
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
      const ElemInfo & elem_info =
          hasBlocks(fi->elemPtr()->subdomain_id()) ? *fi->elemInfo() : *fi->neighborInfo();
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[0]][0];

      // If it is a Dirichlet BC, we use the dirichlet value the make sure the face flux
      // is consistent
      if (_vel[0]->isDirichletBoundaryFace(*fi))
      {
        const Moose::FaceArg boundary_face{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};
        face_rho = _rho(boundary_face, Moose::currentState());

        for (const auto dim_i : make_range(_dim))
          face_hbya(dim_i) =
              -MetaPhysicL::raw_value((*_vel[dim_i])(boundary_face, Moose::currentState()));
      }
      // Otherwise we just do a one-term expansion (so we just use the element value)
      else
      {
        const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[0]][0];

        face_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
        for (const auto dim_i : make_range(_dim))
          face_hbya(dim_i) = hbya_reader[dim_i](elem_dof);
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

    Ainv.pointwise_mult(Ainv, *_cell_volumes);
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
    for (const auto & component : _pressure_system->gradientContainer())
      _grad_p_current.push_back(component->clone());
  }

  return _grad_p_current;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RhieChowMassFlux.h"
#include "INSFVAttributes.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "NS.h"
#include "Assembly.h"
#include "INSFVVelocityVariable.h"
#include "INSFVPressureVariable.h"
#include "PiecewiseByBlockLambdaFunctor.h"
#include "VectorCompositeFunctor.h"
#include "LinearSIMPLE.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem_range.h"
#include "metaphysicl/dualsemidynamicsparsenumberarray.h"

#include "LinearSystem.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", RhieChowMassFlux);

InputParameters
RhieChowMassFlux::validParams()
{
  auto params = GeneralUserObject::validParams();
  params += BlockRestrictable::validParams();
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
  : GeneralUserObject(params),
    BlockRestrictable(this),
    NonADFunctorInterface(this),
    _moose_mesh(UserObject::_subproblem.mesh()),
    _mesh(_moose_mesh.getMesh()),
    _dim(blocksMaxDimension()),
    _p(dynamic_cast<MooseLinearVariableFVReal *>(
        &UserObject::_subproblem.getVariable(0, getParam<VariableName>(NS::pressure)))),
    _vel(_dim, nullptr),
    _HbyA_flux(_moose_mesh, blockIDs(), "HbyA_flux"),
    _Ainv(_moose_mesh, blockIDs(), "Ainv"),
    _face_mass_flux(_moose_mesh, blockIDs(), "face_values"),
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

  if (!dynamic_cast<LinearSIMPLE *>(getMooseApp().getExecutioner()))
    mooseError(this->name(),
               " should only be used with a linear segregated thermal-hydraulics solver!");

  // std::vector<LinearFVFluxKernel *> flux_kernel;
  // auto base_query = _fe_problem.theWarehouse()
  //                       .query()
  //                       .template condition<AttribThread>(_tid)
  //                       .template condition<AttribSysNum>(_p->sys().number())
  //                       .template condition<AttribSystem>("LinearFVFluxKernel")
  //                       .template
  //                       condition<AttribName>(getParam<std::string>("p_diffusion_kernel"))
  //                       .queryInto(flux_kernel);
  // if (flux_kernel.size() != 1)
  //   paramError(
  //       "p_diffusion_kernel",
  //       "The kernel with the given name could not be found or multiple instances were
  //       identified.");
  // _p_diffusion_kernel = dynamic_cast<LinearFVDiffusion *>(flux_kernel[0]);
  // if (!_p_diffusion_kernel)
  //   paramError("p_diffusion_kernel",
  //              "The provided diffusion kernel should of type LinearFVDiffusion!");
}

void
RhieChowMassFlux::linkMomentumPressureSystems(
    std::vector<LinearSystem *> momentum_systems,
    const LinearSystem & pressure_system,
    const std::vector<unsigned int> & momentum_system_numbers)
{
  _momentum_systems = momentum_systems;
  _momentum_system_numbers = momentum_system_numbers;
  _pressure_system = &pressure_system;

  _momentum_implicit_systems.clear();
  for (auto & system : _momentum_systems)
    _momentum_implicit_systems.push_back(dynamic_cast<LinearImplicitSystem *>(&system->system()));
}

void
RhieChowMassFlux::meshChanged()
{
  _HbyA_flux.clear();
  _Ainv.clear();
  _face_mass_flux.clear();
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

  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
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
            fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem};

        const Real face_rho = _rho(boundary_face, time_arg);
        for (const auto dim_i : index_range(_vel))
          density_times_velocity(dim_i) =
              face_rho * raw_value((*_vel[dim_i])(boundary_face, time_arg));
      }

      _face_mass_flux[fi->id()] = density_times_velocity * fi->normal();
    }
  }
}

Real
RhieChowMassFlux::getMassFlux(const FaceInfo & fi) const
{
  return _face_mass_flux.evaluate(&fi);
}

void
RhieChowMassFlux::computeFaceMassFlux()
{
  using namespace Moose::FV;

  const auto time_arg = Moose::currentState();

  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    Real face_area = fi->faceArea() * fi->faceCoord();

    Real p_grad_flux = 0.0;
    if (_p->isInternalFace(*fi))
    {
      const auto p_elem_value = _p->getElemValue(*fi->elemInfo(), time_arg);
      const auto p_neighbor_value = _p->getElemValue(*fi->neighborInfo(), time_arg);

      const auto elem_matrix_contribution = _p_diffusion_kernel->computeElemMatrixContribution();
      const auto neighbor_matrix_contribution =
          _p_diffusion_kernel->computeNeighborMatrixContribution();
      const auto elem_rhs_contribution =
          _p_diffusion_kernel->computeElemRightHandSideContribution();

      p_grad_flux = ((p_neighbor_value * neighbor_matrix_contribution -
                      p_elem_value * elem_matrix_contribution) +
                     elem_rhs_contribution) *
                    face_area;
    }
    else if (auto * bc_pointer = _p->getBoundaryCondition(*fi->boundaryIDs().begin()))
    {
      const auto p_elem_value = _p->getElemValue(*fi->elemInfo(), time_arg);
      const auto matrix_contribution =
          _p_diffusion_kernel->computeBoundaryMatrixContribution(*bc_pointer);
      const auto rhs_contribution =
          _p_diffusion_kernel->computeBoundaryRHSContribution(*bc_pointer);
      p_grad_flux = (p_elem_value * matrix_contribution + rhs_contribution) * face_area;
    }
    _face_mass_flux[fi->id()] = _HbyA_flux[fi->id()] - p_grad_flux;
  }
}

void
RhieChowMassFlux::computeCellVelocity()
{
  auto & pressure_gradient = _pressure_system->gradientContainer();

  // We set the dof value in the solution vector the same logic applies:
  // u_C = -(H/A)_C - (1/A)_C*grad(p)_C where C is the cell index
  for (auto system_i : index_range(_momentum_implicit_systems))
  {
    auto working_vector = _Ainv_raw[system_i]->clone();
    working_vector->pointwise_mult(*working_vector, *pressure_gradient[system_i]);
    working_vector->add(*_HbyA_raw[system_i]);
    (*_momentum_implicit_systems[system_i]->solution) = *working_vector;
    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }
}

void
RhieChowMassFlux::populateHbyA(const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya)
{
  using namespace Moose::FV;
  const auto time_arg = Moose::currentState();

  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
    {
      Real face_rho = 0;
      RealVectorValue face_hbya;
      if (_vel[0]->isInternalFace(*fi))
      {
        const auto & elem_info = *fi->elemInfo();
        const auto & neighbor_info = *fi->neighborInfo();
        const auto elem_dof =
            elem_info.dofIndices()[_momentum_implicit_systems[0]->number()][_vel[0]->number()];
        const auto neighbor_dof =
            neighbor_info.dofIndices()[_momentum_implicit_systems[0]->number()][_vel[0]->number()];

        Real face_rho;
        interpolate(Moose::FV::InterpMethod::Average,
                    face_rho,
                    _rho(makeElemArg(fi->elemPtr()), time_arg),
                    _rho(makeElemArg(fi->neighborPtr()), time_arg),
                    *fi,
                    true);
        for (const auto dim_i : index_range(raw_hbya))
        {
          interpolate(Moose::FV::InterpMethod::Average,
                      face_hbya(dim_i),
                      (*raw_hbya[dim_i])(elem_dof),
                      (*raw_hbya[dim_i])(neighbor_dof),
                      *fi,
                      true);
        }
      }
      else
      {
        const ElemInfo & elem_info =
            hasBlocks(fi->elemPtr()->subdomain_id()) ? *fi->elemInfo() : *fi->neighborInfo();

        if (_vel[0]->isDirichletBoundaryFace(*fi))
        {
          const Moose::FaceArg boundary_face{
              fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem()};
          face_rho = _rho(boundary_face, Moose::currentState());
          for (const auto dim_i : make_range(_dim))
          {
            face_hbya(dim_i) =
                -MetaPhysicL::raw_value((*_vel[dim_i])(boundary_face, Moose::currentState()));
          }
        }
        else
        {
          const auto elem_dof =
              elem_info.dofIndices()[_momentum_implicit_systems[0]->number()][_vel[0]->number()];

          face_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
          for (const auto dim_i : make_range(_dim))
            face_hbya(dim_i) = (*raw_hbya[dim_i])(elem_dof);
        }
      }
      _HbyA_flux[fi->id()] *= face_hbya * fi->normal() * face_rho;
    }
  }
}

void
RhieChowMassFlux::populateAinv(const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_Ainv)
{
  using namespace Moose::FV;

  const auto time_arg = Moose::currentState();

  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
    {
      // If we are on an internal face, we just interpolate the values to the faces.
      // Otherwise, depending on the boundary type, we take the velocity value or
      // extrapolated HbyA values.
      if (_vel[0]->isInternalFace(*fi))
      {
        const auto & elem_info = *fi->elemInfo();
        const auto & neighbor_info = *fi->neighborInfo();
        const auto elem_dof =
            elem_info.dofIndices()[_momentum_implicit_systems[0]->number()][_vel[0]->number()];
        const auto neighbor_dof =
            neighbor_info.dofIndices()[_momentum_implicit_systems[0]->number()][_vel[0]->number()];

        Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
        Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);

        for (const auto dim_i : index_range(raw_Ainv))
          interpolate(InterpMethod::Average,
                      _Ainv[fi->id()](dim_i),
                      elem_rho * (*raw_Ainv[dim_i])(elem_dof)*elem_info.volume() *
                          elem_info.coordFactor(),
                      neighbor_rho * (*raw_Ainv[dim_i])(neighbor_dof)*neighbor_info.volume() *
                          neighbor_info.coordFactor(),
                      *fi,
                      true);
      }
      else
      {
        const ElemInfo & elem_info =
            hasBlocks(fi->elemPtr()->subdomain_id()) ? *fi->elemInfo() : *fi->neighborInfo();
        const auto elem_dof =
            elem_info.dofIndices()[_momentum_implicit_systems[0]->number()][_vel[0]->number()];

        Real elem_rho = _rho(makeElemArg(elem_info.elem()), time_arg);

        for (const auto dim_i : index_range(raw_Ainv))
          _Ainv[fi->id()](dim_i) =
              elem_rho * (*raw_Ainv[dim_i])(elem_dof)*elem_info.volume() * elem_info.coordFactor();
      }
    }
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

  auto & pressure_gradient = _pressure_system->gradientContainer();

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

    _Ainv_raw.push_back(current_local_solution.zero_clone());
    NumericVector<Number> & Ainv = *(_Ainv_raw.back());

    mmat->get_diagonal(Ainv);

    auto working_vector = momentum_system->current_local_solution->zero_clone();
    PetscVector<Number> * working_vector_petsc =
        dynamic_cast<PetscVector<Number> *>(working_vector.get());
    mooseAssert(working_vector_petsc,
                "The vectors used in the RhieChowMassFlux objects need to be convertable "
                "to PetscVectors!");

    *working_vector_petsc = 1.0;
    Ainv.pointwise_divide(*working_vector_petsc, Ainv);

    if (verbose)
    {
      _console << "Velocity solution in H(u)" << std::endl;
      solution.print();
    }

    _HbyA_raw.push_back(current_local_solution.zero_clone());
    NumericVector<Number> & HbyA = *(_HbyA_raw.back());
    HbyA = 0;
    HbyA.add(rhs);

    // Now we set the diagonal of our system matrix to 0 so we can create H*u
    // TODO: Add a function for this in libmesh
    *working_vector_petsc = 0.0;
    MatDiagonalSet(mmat->mat(), working_vector_petsc->vec(), INSERT_VALUES);

    if (verbose)
    {
      _console << "H" << std::endl;
      mmat->print();
    }

    if (verbose)
    {
      _console << "total RHS" << std::endl;
      rhs.print();
      _console << "pressure RHS" << std::endl;
      pressure_gradient[system_i]->print();
    }

    // We correct the right hand side to exclude the pressure contribution
    HbyA.add(-1.0, *pressure_gradient[system_i]);

    if (verbose)
    {
      _console << "H RHS" << std::endl;
      HbyA.print();
    }

    // Create H(u)
    mmat->vector_mult(*working_vector_petsc, solution);

    if (verbose)
    {
      _console << " H(u)" << std::endl;
      working_vector_petsc->print();
    }

    // Create H(u) - RHS
    HbyA.add(*working_vector_petsc);

    if (verbose)
    {
      _console << " H(u)-rhs-relaxation_source" << std::endl;
      HbyA.print();
    }

    // Create 1/A*(H(u)-RHS)
    HbyA.pointwise_mult(HbyA, Ainv);

    if (verbose)
    {
      _console << " (H(u)-rhs)/A" << std::endl;
      HbyA.print();
    }
  }

  // We fill the 1/A functor
  populateAinv(_Ainv_raw);
  populateHbyA(_HbyA_raw);

  if (verbose)
  {
    _console << "************************************" << std::endl;
    _console << "DONE Computing HbyA " << std::endl;
    _console << "************************************" << std::endl;
  }
}

bool
RhieChowMassFlux::hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const
{
  if (fi_elem_side)
    return hasBlocks(fi.elem().subdomain_id());
  else
    return fi.neighborPtr() && hasBlocks(fi.neighbor().subdomain_id());
}

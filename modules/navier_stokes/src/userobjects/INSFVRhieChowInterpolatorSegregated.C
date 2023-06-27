//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVRhieChowInterpolatorSegregated.h"
#include "INSFVAttributes.h"
#include "GatherRCDataElementThread.h"
#include "GatherRCDataFaceThread.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "SystemBase.h"
#include "NS.h"
#include "Assembly.h"
#include "INSFVVelocityVariable.h"
#include "INSFVPressureVariable.h"
#include "PiecewiseByBlockLambdaFunctor.h"
#include "VectorCompositeFunctor.h"
#include "FVElementalKernel.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem_range.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/remote_elem.h"
#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "timpi/parallel_sync.h"

#include "NonlinearSystem.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", INSFVRhieChowInterpolatorSegregated);

InputParameters
INSFVRhieChowInterpolatorSegregated::validParams()
{
  auto params = RhieChowInterpolatorBase::validParams();

  params.addClassDescription("Computes H/A and 1/A together with face velocities for segregated "
                             "momentum-pressure equations.");

  // We diable the execution of this, should only provide functions
  // for the executioner
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_NONE);
  exec_enum = {EXEC_NONE};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

INSFVRhieChowInterpolatorSegregated::INSFVRhieChowInterpolatorSegregated(
    const InputParameters & params)
  : RhieChowInterpolatorBase(params),
    _HbyA(_moose_mesh, _sub_ids, "HbyA"),
    _Ainv(_moose_mesh, _sub_ids, "Ainv", false),
    _vel(std::make_unique<PiecewiseByBlockLambdaFunctor<ADRealVectorValue>>(
        name(),
        [this](const auto & r, const auto & t) -> ADRealVectorValue
        {
          ADRealVectorValue velocity((*_u)(r, t));
          if (_dim >= 2)
            velocity(1) = (*_v)(r, t);
          if (_dim >= 3)
            velocity(2) = (*_w)(r, t);
          return velocity;
        },
        std::set<ExecFlagType>({EXEC_ALWAYS}),
        _moose_mesh,
        blockIDs())),
    _face_velocity(_moose_mesh, _sub_ids, "face_values")

{
  // Register the elemental/face functors which will be queried in the pressure equation
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("Ainv", _Ainv, tid);
    UserObject::_subproblem.addFunctor("HbyA", _HbyA, tid);
  }
}

void
INSFVRhieChowInterpolatorSegregated::linkMomentumSystem(
    std::vector<NonlinearSystemBase *> momentum_systems,
    const std::vector<unsigned int> & momentum_system_numbers,
    const TagID & pressure_gradient_tag)
{
  _momentum_systems = momentum_systems;
  _momentum_system_numbers = momentum_system_numbers;
  _pressure_gradient_tag = pressure_gradient_tag;

  _momentum_implicit_systems.clear();
  for (auto & system : _momentum_systems)
    _momentum_implicit_systems.push_back(
        dynamic_cast<NonlinearImplicitSystem *>(&system->system()));
}

void
INSFVRhieChowInterpolatorSegregated::meshChanged()
{
  _HbyA.clear();
  _Ainv.clear();
  _face_velocity.clear();
}

void
INSFVRhieChowInterpolatorSegregated::initialize()
{
  for (const auto & pair : _HbyA)
    _HbyA[pair.first] = 0;

  for (const auto & pair : _Ainv)
    _Ainv[pair.first] = 0;
}

void
INSFVRhieChowInterpolatorSegregated::initFaceVelocities()
{
  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    // On internal face we do a regular interpoaltion with geometric weights
    if (_u->isInternalFace(*fi))
    {
      const Moose::FaceArg face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr};

      _face_velocity[fi->id()] = raw_value((*_vel)(face, Moose::currentState()));
    }
    // On the boundary, we just take the boundary values
    else
    {
      const Elem * const boundary_elem =
          hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->elemPtr() : fi->neighborPtr();

      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem};

      _face_velocity[fi->id()] = raw_value((*_vel)(boundary_face, Moose::currentState()));
    }
  }
}

VectorValue<ADReal>
INSFVRhieChowInterpolatorSegregated::getVelocity(const FaceInfo & fi,
                                                 const Moose::StateArg & /*time*/,
                                                 THREAD_ID /*tid*/,
                                                 Moose::FV::InterpMethod /*m*/) const
{
  return _face_velocity.evaluate(&fi);
}

void
INSFVRhieChowInterpolatorSegregated::computeFaceVelocity()
{
  const auto time_arg = Moose::currentState();

  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    // On internal face we just use the interpoalted H/A and the pressure face gradient
    // So u_f = -(H/A)_f - (1/A)_f*grad(p)_f
    // Notice the (-) sign on H/A which comes from the face that we use the Jacobian/Residual
    // computations and we get -H instead of H.
    if (_u->isInternalFace(*fi))
    {
      const Moose::FaceArg face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr};

      Real Ainv;
      RealVectorValue HbyA = raw_value(_HbyA(face, time_arg));

      interpolate(Moose::FV::InterpMethod::Average,
                  Ainv,
                  _Ainv(makeElemArg(fi->elemPtr()), time_arg),
                  _Ainv(makeElemArg(fi->neighborPtr()), time_arg),
                  *fi,
                  true);

      RealVectorValue grad_p = raw_value(_p->gradient(face, time_arg));
      for (const auto comp_index : make_range(_dim))
        _face_velocity[fi->id()](comp_index) = -HbyA(comp_index) - Ainv * grad_p(comp_index);
    }
    else
    {
      const Elem * const boundary_elem =
          hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->elemPtr() : fi->neighborPtr();
      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem};

      // If we have a dirichlet boundary conditions, this sill give us the exact value of the
      // velocity on the face as expected (see populateHbyA())
      if (_u->isDirichletBoundaryFace(*fi, boundary_elem, time_arg))
        _face_velocity[fi->id()] = -raw_value(_HbyA(boundary_face, time_arg));
      else
      {
        const Real & Ainv = raw_value(_Ainv(boundary_face, time_arg));
        const RealVectorValue & HbyA = raw_value(_HbyA(boundary_face, time_arg));
        const RealVectorValue & grad_p = raw_value(_p->gradient(boundary_face, time_arg));
        for (const auto comp_index : make_range(_dim))
          _face_velocity[fi->id()](comp_index) = -HbyA(comp_index) - Ainv * grad_p(comp_index);
      }
    }
  }
}

void
INSFVRhieChowInterpolatorSegregated::computeCellVelocity()
{
  bool segregated_systems = _momentum_implicit_systems.size() > 1;
  std::vector<unsigned int> var_nums = {_momentum_implicit_systems[0]->variable_number(_u->name())};
  if (_v)
  {
    unsigned int system_index = segregated_systems ? 1 : 0;
    var_nums.push_back(_momentum_implicit_systems[system_index]->variable_number(_v->name()));
  }
  if (_w)
  {
    unsigned int system_index = segregated_systems ? 2 : 0;
    var_nums.push_back(_momentum_implicit_systems[system_index]->variable_number(_w->name()));
  }

  const auto time_arg = Moose::currentState();

  for (auto & elem :
       as_range(_mesh.active_local_elements_begin(), _mesh.active_local_elements_end()))
  {
    const auto elem_arg = makeElemArg(elem);
    const Real Ainv = _Ainv(elem_arg, time_arg);
    const RealVectorValue & grad_p = raw_value(_p->gradient(elem_arg, time_arg));

    for (auto comp_index : make_range(_dim))
    {
      // If we are doing segregated momentum components we need to access different vector
      // components otherwise everything is in the same vector (with different variable names)
      unsigned int hbya_index = segregated_systems ? comp_index : 0;
      unsigned int system_number = segregated_systems
                                       ? _momentum_implicit_systems[comp_index]->number()
                                       : _momentum_implicit_systems[0]->number();

      const auto index = elem->dof_number(system_number, var_nums[comp_index], 0);

      // We set the dof value in the solution vector the same logic applies:
      // u_C = -(H/A)_C - (1/A)_C*grad(p)_C where C is the cell index
      _momentum_implicit_systems[hbya_index]->solution->set(
          index, -(*_HbyA_raw[hbya_index])(index)-Ainv * grad_p(comp_index));
    }
  }

  for (auto system_i : index_range(_momentum_implicit_systems))
  {
    _momentum_implicit_systems[system_i]->solution->close();
    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }
}

void
INSFVRhieChowInterpolatorSegregated::populateHbyA(
    const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya,
    const std::vector<unsigned int> & var_nums)
{
  bool segregated_systems = raw_hbya.size() > 1;
  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    // If we are on an internal face, we just interpolate the values to the faces.
    // Otherwise, depending on the boundary type, we take the velocity value or
    // extrapolated HbyA values.
    if (_u->isInternalFace(*fi))
    {
      RealVectorValue HbyA;
      const Elem * elem = fi->elemPtr();
      const Elem * neighbor = fi->neighborPtr();
      for (auto comp_index : make_range(_dim))
      {
        // If we are not segregated in momentum components, every variable is
        // in the same vector
        unsigned int hbya_index = segregated_systems ? comp_index : 0;
        unsigned int system_number = _momentum_implicit_systems[hbya_index]->number();
        const auto dof_index_elem = elem->dof_number(system_number, var_nums[comp_index], 0);
        const auto dof_index_neighbor =
            neighbor->dof_number(system_number, var_nums[comp_index], 0);

        _HbyA[fi->id()](comp_index) = 0.0;
        interpolate(Moose::FV::InterpMethod::Average,
                    _HbyA[fi->id()](comp_index),
                    (*raw_hbya[hbya_index])(dof_index_elem),
                    (*raw_hbya[hbya_index])(dof_index_neighbor),
                    *fi,
                    true);
      }
    }
    else
    {
      const Elem * const boundary_elem =
          hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->elemPtr() : fi->neighborPtr();

      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem};

      if (_u->isDirichletBoundaryFace(*fi, boundary_elem, Moose::currentState()))
        _HbyA[fi->id()] = -raw_value((*_vel)(boundary_face, Moose::currentState()));
      else
        for (const auto comp_index : make_range(_dim))
        {
          unsigned int hbya_index = segregated_systems ? comp_index : 0;
          unsigned int system_number = _momentum_implicit_systems[hbya_index]->number();
          const auto dof_index_elem =
              boundary_elem->dof_number(system_number, var_nums[comp_index], 0);
          _HbyA[fi->id()](comp_index) = (*raw_hbya[hbya_index])(dof_index_elem);
        }
    }
  }
}

void
INSFVRhieChowInterpolatorSegregated::computeHbyA(bool verbose)
{
  if (verbose)
  {
    std::cout << "************************************" << std::endl;
    std::cout << "Computing HbyA" << std::endl;
    std::cout << "************************************" << std::endl;
  }
  mooseAssert(_momentum_implicit_systems.size() && _momentum_implicit_systems[0],
              "The momentum system shall be linked before calling this function!");

  NonlinearImplicitSystem * momentum_system = _momentum_implicit_systems[0];

  std::vector<unsigned int> var_nums = {_momentum_implicit_systems[0]->variable_number(_u->name())};
  if (_v)
  {
    unsigned int system_index = _momentum_systems.size() > 1 ? 1 : 0;
    var_nums.push_back(_momentum_implicit_systems[system_index]->variable_number(_v->name()));
  }
  if (_w)
  {
    unsigned int system_index = _momentum_systems.size() > 1 ? 2 : 0;
    var_nums.push_back(_momentum_implicit_systems[system_index]->variable_number(_w->name()));
  }

  // We will need the petsc interface in this case, until the libmesh functions are there
  PetscMatrix<Number> * mmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system->matrix);

  // Data structure to manipulate 1/A_i where A_i is the diagonal of the momentum system matrix
  std::unique_ptr<NumericVector<Number>> Ainv =
      momentum_system->current_local_solution->zero_clone();
  PetscVector<Number> * Ainv_petsc = dynamic_cast<PetscVector<Number> *>(Ainv.get());

  // A working vector which will be used as a placeholder for partial results
  std::unique_ptr<NumericVector<Number>> working_vector =
      momentum_system->current_local_solution->zero_clone();
  PetscVector<Number> * working_vector_petsc =
      dynamic_cast<PetscVector<Number> *>(working_vector.get());

  if (verbose)
  {
    std::cout << "Matrix in rc object" << std::endl;
    mmat->print();
  }

  // Populating 1/A first
  mmat->get_diagonal(*Ainv_petsc);

  if (verbose)
  {
    std::cout << "A" << std::endl;
    Ainv_petsc->print();
  }

  *working_vector_petsc = 1.0;
  Ainv_petsc->pointwise_divide(*working_vector_petsc, *Ainv_petsc);

  // We fill the 1/A functor
  auto active_local_begin =
      _mesh.evaluable_elements_begin(momentum_system->get_dof_map(), var_nums[0]);
  auto active_local_end = _mesh.evaluable_elements_end(momentum_system->get_dof_map(), var_nums[0]);
  for (auto it = active_local_begin; it != active_local_end; ++it)
  {
    const Elem * elem = *it;
    if (this->hasBlocks(elem->subdomain_id()))
    {
      const Real volume = _moose_mesh.elemInfo(elem->id()).volume();
      const auto dof_index = elem->dof_number(momentum_system->number(), var_nums[0], 0);
      _Ainv[elem->id()] = (*Ainv_petsc)(dof_index)*volume;
    }
  }

  // Now we set the diagonal of our system matrix to 0 so we can create H*u
  // TODO: Add a function for this in libmesh
  *working_vector_petsc = 0.0;
  MatDiagonalSet(mmat->mat(), working_vector_petsc->vec(), INSERT_VALUES);

  if (verbose)
  {
    std::cout << "H" << std::endl;
    mmat->print();
  }

  _HbyA_raw.clear();
  for (auto system_i : index_range(_momentum_systems))
  {
    _fe_problem.setCurrentNonlinearSystem(_momentum_system_numbers[system_i]);

    momentum_system = _momentum_implicit_systems[system_i];

    NumericVector<Number> & rhs = *(momentum_system->rhs);
    NumericVector<Number> & current_local_solution = *(momentum_system->current_local_solution);
    NumericVector<Number> & solution = *(momentum_system->solution);

    _HbyA_raw.push_back(current_local_solution.zero_clone());
    NumericVector<Number> & HbyA = *(_HbyA_raw.back());

    if (verbose)
    {
      std::cout << "Velocity solution in H(u)" << std::endl;
      solution.print();
    }

    // We need to subtract the contribution of the pressure gradient from the residual (right hand
    // side). We plug working_vector=0 in here to ge the right hand side contribution
    _fe_problem.computeResidualTag(*working_vector, HbyA, _pressure_gradient_tag);

    // Now we reorganize the association between tags and vectors to make sure
    // that the next solve is perfect
    _momentum_systems[system_i]->associateVectorToTag(
        momentum_system->get_vector(_fe_problem.vectorTagName(0)), 0);
    _momentum_systems[system_i]->associateVectorToTag(
        momentum_system->get_vector(_fe_problem.vectorTagName(_pressure_gradient_tag)),
        _pressure_gradient_tag);

    if (verbose)
    {
      std::cout << "total RHS" << std::endl;
      rhs.print();
      std::cout << "pressure RHS" << std::endl;
      HbyA.print();
    }

    // We correct the right hand side to exclude the pressure contribution
    HbyA.scale(-1.0);
    HbyA.add(-1.0, rhs);

    if (verbose)
    {
      std::cout << "H RHS" << std::endl;
      HbyA.print();
    }

    // Create H(u)
    mmat->vector_mult(*working_vector_petsc, solution);

    if (verbose)
    {
      std::cout << " H(u)" << std::endl;
      working_vector_petsc->print();
    }

    // Create H(u) - RHS
    HbyA.add(*working_vector_petsc);

    if (verbose)
    {
      std::cout << " H(u)-rhs-relaxation_source" << std::endl;
      HbyA.print();
    }

    // Create 1/A*(H(u)-RHS)
    HbyA.pointwise_mult(HbyA, *Ainv);

    if (verbose)
    {
      std::cout << " (H(u)-rhs)/A" << std::endl;
      HbyA.print();
    }
  }

  populateHbyA(_HbyA_raw, var_nums);

  if (verbose)
  {
    std::cout << "************************************" << std::endl;
    std::cout << "DONE Computing HbyA " << std::endl;
    std::cout << "************************************" << std::endl;
  }
}

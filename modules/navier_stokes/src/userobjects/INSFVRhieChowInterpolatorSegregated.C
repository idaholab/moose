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

  params.addClassDescription(
      "Computes the Rhie-Chow velocity based on gathered 'a' coefficient data.");

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
  // Register the elemental functors which will be queried in the pressure equation
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("Ainv", _Ainv, tid);
    UserObject::_subproblem.addFunctor("HbyA", _HbyA, tid);
  }
}

void
INSFVRhieChowInterpolatorSegregated::linkMomentumSystem(NonlinearSystemBase & momentum_system,
                                                        const TagID & momentum_tag)
{
  _momentum_sys = &momentum_system;
  _momentum_tag = momentum_tag;
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
    if (_u->isInternalFace(*fi))
    {
      const Moose::FaceArg face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr};

      _face_velocity[fi->id()] = raw_value((*_vel)(face, Moose::currentState()));
      // _console << "Computing initial face velocity " << fi->id() <<
      // raw_value(_face_velocity(face))
      //          << std::endl;
    }
    else if (Moose::FV::onBoundary(*_u, *fi))
    {
      const Elem * const boundary_elem =
          hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->elemPtr() : fi->neighborPtr();

      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem};

      _face_velocity[fi->id()] = raw_value((*_vel)(boundary_face, Moose::currentState()));
      // _console << "Computing initial face velocity " << fi->id()
      //          << raw_value(_face_velocity(boundary_face)) << std::endl;
    }
  }
}

void
INSFVRhieChowInterpolatorSegregated::execute()
{
  // computeHbyA();
}

VectorValue<ADReal>
INSFVRhieChowInterpolatorSegregated::getVelocity(const FaceInfo & fi,
                                                 const Moose::StateArg & /*time*/,
                                                 THREAD_ID /*tid*/,
                                                 Moose::FV::InterpMethod /*m*/) const
{
  // // const bool correct_skewness =
  // //     (_u->faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage);

  // // if (Moose::FV::onBoundary(*this, fi))
  // // {
  // //   const Elem * const boundary_elem =
  // //       hasBlocks(fi.elemPtr()->subdomain_id()) ? fi.elemPtr() : fi.neighborPtr();

  // //   const Moose::FaceArg boundary_face{
  // //       &fi, Moose::FV::LimiterType::CentralDifference, true, correct_skewness, boundary_elem};

  // //   _console << "returning: " << (*_vel)(boundary_face)(0) << std::endl;
  // //   return (*_vel)(boundary_face);
  // // }

  // _console << "returning: " << raw_value(_face_velocity.evaluate(&fi)) << std::endl;
  return _face_velocity.evaluate(&fi);
}

void
INSFVRhieChowInterpolatorSegregated::computeVelocity(const Real & momentum_relaxation)
{
  NonlinearImplicitSystem & momentum_system =
      dynamic_cast<NonlinearImplicitSystem &>(_momentum_sys->system());

  PetscMatrix<Number> * pmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system.matrix);
  PetscVector<Number> * solution =
      dynamic_cast<PetscVector<Number> *>(momentum_system.current_local_solution.get());

  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    if (_u->isInternalFace(*fi))
    {
      const Moose::FaceArg face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr};

      RealVectorValue Ainv; //  = raw_value(_Ainv(face));
      RealVectorValue HbyA = raw_value(_HbyA(face, Moose::currentState()));

      interpolate(Moose::FV::InterpMethod::Average,
                  Ainv,
                  _Ainv(makeElemArg(fi->elemPtr()), Moose::currentState()),
                  _Ainv(makeElemArg(fi->neighborPtr()), Moose::currentState()),
                  *fi,
                  true);

      // interpolate(Moose::FV::InterpMethod::Average,
      //             HbyA,
      //             _HbyA(makeElemArg(fi->elemPtr())),
      //             _HbyA(makeElemArg(fi->neighborPtr())),
      //             *fi,
      //             true);

      RealVectorValue grad_p = raw_value(_p->gradient(face, Moose::currentState()));
      for (const auto comp_index : make_range(_dim))
      {
        _face_velocity[fi->id()](comp_index) =
            -HbyA(comp_index) - Ainv(comp_index) * grad_p(comp_index);
      }
      // _console << "ID " << fi->id() << raw_value(_face_velocity(face)) << std::endl;
    }
    else if (Moose::FV::onBoundary(*_u, *fi))
    {
      const Elem * const boundary_elem =
          hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->elemPtr() : fi->neighborPtr();

      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem};

      if (_u->isDirichletBoundaryFace(*fi, boundary_elem, Moose::currentState()))
      {
        _face_velocity[fi->id()] = -raw_value(
            _HbyA(boundary_face, Moose::currentState())); // raw_value((*_vel)(boundary_face));
        // _console << "ID " << fi->id() << raw_value(_face_velocity(boundary_face)) << std::endl;
      }
      else
      {
        RealVectorValue Ainv = raw_value(_Ainv(boundary_face, Moose::currentState()));
        RealVectorValue HbyA = raw_value(_HbyA(boundary_face, Moose::currentState()));
        RealVectorValue grad_p = raw_value(_p->gradient(boundary_face, Moose::currentState()));
        for (const auto comp_index : make_range(_dim))
        {
          _face_velocity[fi->id()](comp_index) =
              -HbyA(comp_index) - Ainv(comp_index) * grad_p(comp_index);
        }
        // _console << "ID " << fi->id() << raw_value(_face_velocity(boundary_face)) << std::endl;
      }
    }
  }
}

void
INSFVRhieChowInterpolatorSegregated::populateHbyA(NonlinearImplicitSystem & momentum_system,
                                                  const NumericVector<Number> & raw_hbya,
                                                  const std::vector<unsigned int> & var_nums)
{
  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    if (_u->isInternalFace(*fi))
    {
      RealVectorValue HbyA; //  = raw_value(_HbyA(face));

      const Elem * elem = fi->elemPtr();
      const Elem * neighbor = fi->neighborPtr();
      for (auto comp_index : make_range(_dim))
      {
        const auto dof_index_elem =
            elem->dof_number(momentum_system.number(), var_nums[comp_index], 0);
        const auto dof_index_neighbor =
            neighbor->dof_number(momentum_system.number(), var_nums[comp_index], 0);

        _HbyA[fi->id()](comp_index) = 0.0;
        interpolate(Moose::FV::InterpMethod::Average,
                    _HbyA[fi->id()](comp_index),
                    raw_hbya(dof_index_elem),
                    raw_hbya(dof_index_neighbor),
                    *fi,
                    true);
      }
    }
    else if (Moose::FV::onBoundary(*_u, *fi))
    {
      const Elem * const boundary_elem =
          hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->elemPtr() : fi->neighborPtr();

      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem};

      if (_u->isDirichletBoundaryFace(*fi, boundary_elem, Moose::currentState()))
      {
        _HbyA[fi->id()] = -raw_value((*_vel)(boundary_face, Moose::currentState()));
        // _console << "ID " << fi->id() << raw_value(_face_velocity(boundary_face)) << std::endl;
      }
      else
      {
        for (const auto comp_index : make_range(_dim))
        {
          const auto dof_index_elem =
              boundary_elem->dof_number(momentum_system.number(), var_nums[comp_index], 0);
          _HbyA[fi->id()](comp_index) = raw_hbya(dof_index_elem);
        }
      }
    }
  }
}

void
INSFVRhieChowInterpolatorSegregated::computeHbyA(const Real & momentum_relaxation,
                                                 const bool verbose)
{
  mooseAssert(_momentum_sys, "The momentum system shall be linked before calling this function!");

  NonlinearImplicitSystem & momentum_system =
      dynamic_cast<NonlinearImplicitSystem &>(_momentum_sys->system());

  PetscMatrix<Number> * mmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system.matrix);
  PetscVector<Number> * mrhs = dynamic_cast<PetscVector<Number> *>(momentum_system.rhs);
  PetscVector<Number> * solution =
      dynamic_cast<PetscVector<Number> *>(momentum_system.current_local_solution.get());

  std::unique_ptr<NumericVector<Number>> Ainv = solution->zero_clone();
  std::unique_ptr<NumericVector<Number>> HbyA = solution->zero_clone();
  std::unique_ptr<NumericVector<Number>> working_vector = solution->zero_clone();

  // Wpuld be cool to add asserts here
  PetscVector<Number> * Ainv_petsc = dynamic_cast<PetscVector<Number> *>(Ainv.get());
  PetscVector<Number> * HbyA_petsc = dynamic_cast<PetscVector<Number> *>(HbyA.get());
  PetscVector<Number> * working_vector_petsc =
      dynamic_cast<PetscVector<Number> *>(working_vector.get());

  // // **************************************************************************
  // // PETSc version
  // // **************************************************************************

  // We compute the right hand side with a zero vector for starters. This is the right
  // hand side of the linearized system and will be the basis of the HbyA vector.
  _fe_problem.computeResidualTag(*working_vector.get(), *HbyA.get(), _momentum_tag);
  _momentum_sys->associateVectorToTag(
      momentum_system.system().get_vector(_fe_problem.vectorTagName(0)), 0);

  _momentum_sys->associateVectorToTag(
      momentum_system.system().get_vector(_fe_problem.vectorTagName(_momentum_tag)), _momentum_tag);

  _momentum_sys->setSolution(*(_momentum_sys->currentSolution()));

  NumericVector<Number> * old_solution = dynamic_cast<PetscVector<Number> *>(
      &_momentum_sys->getVector(Moose::PREVIOUS_NL_SOLUTION_TAG));

  _momentum_sys->setSolution(*solution);

  if (verbose)
  {
    std::cout << "Velocity solution" << std::endl;
    solution->print();
  }

  // if (pmat->closed())
  // {
  //   // We compute the right hand side with a zero vector for starters
  //   _fe_problem.computeResidualSys(momentum_system, *working_vector, *HbyA);

  //   pmat->print_personal();

  //   VecScale(HbyA_petsc->vec(), -1.0);

  //   MatGetDiagonal(pmat->mat(), Ainv_petsc->vec());

  //   // Now the hard part, let's get H*u and add it to the rhs
  //   MatMult(pmat->mat(), solution->vec(), working_vector_petsc->vec());
  //   VecAXPY(HbyA_petsc->vec(), 1.0, working_vector_petsc->vec());

  //   // We subtract the diagonal component because we didn't want to modify the matrix
  //   VecPointwiseMult(working_vector_petsc->vec(), Ainv_petsc->vec(), solution->vec());
  //   VecAXPY(HbyA_petsc->vec(), -1.0, working_vector_petsc->vec());

  //   // Now we invert the diagonal
  //   VecSet(working_vector_petsc->vec(), 1.0);
  //   VecPointwiseDivide(Ainv_petsc->vec(), working_vector_petsc->vec(), Ainv_petsc->vec());

  //   // And divide H by the diagonal
  //   VecPointwiseMult(HbyA_petsc->vec(), HbyA_petsc->vec(), Ainv_petsc->vec());
  // }

  // **************************************************************************
  // Libmesh version
  // **************************************************************************

  if (verbose)
  {
    std::cout << "Matrix" << std::endl;
    mmat->print();
  }
  mmat->get_diagonal(*Ainv_petsc);

  if (verbose)
  {
    std::cout << "A" << std::endl;
    Ainv_petsc->print();
  }
  MatDiagonalSet(mmat->mat(), working_vector_petsc->vec(), INSERT_VALUES);

  if (verbose)
  {
    std::cout << "total RHS" << std::endl;
    mrhs->print();
  }

  HbyA->add(-1.0, *mrhs);

  if (verbose)
  {
    std::cout << "H RHS" << std::endl;
    HbyA->print();
  }

  // solution->scale(relax);
  // solution->add(1 - relax, *old_solution);

  // solution->print();
  // solution->close();

  if (verbose)
  {
    std::cout << "H" << std::endl;
    mmat->print();
  }

  mmat->vector_mult(*working_vector_petsc, *solution);

  if (verbose)
  {
    std::cout << " H(u)" << std::endl;
    working_vector_petsc->print();
  }

  HbyA_petsc->add(*working_vector_petsc);

  if (verbose)
  {
    std::cout << " H(u)-rhs" << std::endl;
    HbyA->print();
  }

  // HbyA_petsc->add(*working_vector_petsc);

  if (verbose)
  {
    std::cout << " H(u)-rhs-relaxation_source" << std::endl;
    HbyA_petsc->print();
  }

  *working_vector_petsc = 1.0;
  VecPointwiseDivide(Ainv_petsc->vec(), working_vector_petsc->vec(), Ainv_petsc->vec());
  HbyA_petsc->pointwise_mult(*HbyA_petsc, *Ainv_petsc);

  if (verbose)
  {
    std::cout << " (H(u)-rhs)/A" << std::endl;
    HbyA_petsc->print();
  }

  // **************************************************************************
  // Populating the
  // **************************************************************************

  auto begin = _mesh.active_local_elements_begin();
  auto end = _mesh.active_local_elements_end();

  std::vector<unsigned int> var_nums = {momentum_system.variable_number(_u->name())};
  if (_v)
    var_nums.push_back(momentum_system.variable_number(_v->name()));
  if (_w)
    var_nums.push_back(momentum_system.variable_number(_w->name()));

  for (auto it = begin; it != end; ++it)
  {
    const Elem * elem = *it;
    if (this->hasBlocks(elem->subdomain_id()))
    {
      for (auto comp_index : make_range(_dim))
      {
        const Real volume = _moose_mesh.elemInfo(elem->id()).volume();
        const auto dof_index = elem->dof_number(momentum_system.number(), var_nums[comp_index], 0);
        _Ainv[elem->id()](comp_index) = (*Ainv)(dof_index)*volume;
      }
    }
  }

  populateHbyA(momentum_system, *HbyA, var_nums);

  // **************************************************************************
  // END
  // **************************************************************************

  // auto evaluable_begin = _mesh.evaluable_elements_begin(sys.dofMap());
  // auto evaluable_end = _mesh.evaluable_elements_end(sys.dofMap());

  // for (auto it = evaluable_begin; it != evaluable_end; ++it)
  // {
  //   const Elem * elem = *it;

  //   for (unsigned int var_index = 0; var_index < _var_numbers.size(); var_index++)
  //   {
  //     dof_id_type dof_index = elem->dof_number(sys.number(), _var_numbers[var_index], 0);
  //     _HbyA[elem->id()](var_index) = HbyA_petsc->el(dof_index);
  //     _Ainv[elem->id()](var_index) = Ainv_petsc->el(dof_index);
  //   }
  // }
}

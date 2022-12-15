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
    _Ainv(_moose_mesh, _sub_ids, "Ainv"),
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
        blockIDs()))
{
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
INSFVRhieChowInterpolatorSegregated::execute()
{
  computeHbyA();
}

VectorValue<ADReal>
INSFVRhieChowInterpolatorSegregated::getVelocity(const FaceInfo & fi,
                                                 const Moose::StateArg & /*time*/,
                                                 THREAD_ID /*tid*/,
                                                 Moose::FV::InterpMethod /*m*/) const
{
  if (Moose::FV::onBoundary(*this, fi))
  {
    const Elem * const elem = &fi.elem();
    const Elem * const neighbor = fi.neighborPtr();
    const bool correct_skewness =
        (_u->faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage);

    const auto sub_id =
        hasBlocks(elem->subdomain_id()) ? elem->subdomain_id() : neighbor->subdomain_id();
    const Moose::SingleSidedFaceArg boundary_face{
        &fi, Moose::FV::LimiterType::CentralDifference, true, correct_skewness, sub_id};

    _console << "returning: " << (*_vel)(boundary_face)(0) << std::endl;
    return (*_vel)(boundary_face);
  }

  _console << "returning: " << ADRealVectorValue(0.5)(0) << std::endl;
  return ADRealVectorValue(0.5);
}

void
INSFVRhieChowInterpolatorSegregated::computeHbyA()
{
  mooseAssert(_momentum_sys, "The momentum system shall be linked before calling this function!");

  NonlinearImplicitSystem & momentum_system =
      dynamic_cast<NonlinearImplicitSystem &>(_momentum_sys->system());

  PetscMatrix<Number> * pmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system.matrix);
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

  HbyA->print();

  _momentum_sys->setSolution(*solution);

  solution->print();

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

  pmat->print();
  pmat->get_diagonal(*Ainv_petsc);

  Ainv_petsc->print();
  MatDiagonalSet(pmat->mat(), working_vector_petsc->vec(), INSERT_VALUES);

  pmat->print();

  pmat->vector_mult(*working_vector_petsc, *solution);

  working_vector_petsc->print();
  HbyA_petsc->print();

  HbyA_petsc->add(*working_vector_petsc);

  HbyA_petsc->print();

  *working_vector_petsc = 1.0;
  VecPointwiseDivide(Ainv_petsc->vec(), working_vector_petsc->vec(), Ainv_petsc->vec());
  HbyA_petsc->pointwise_mult(*HbyA_petsc, *Ainv_petsc);

  HbyA_petsc->print();

  // **************************************************************************
  // Populating the
  // **************************************************************************

  auto begin = _mesh.active_local_elements_begin();
  auto end = _mesh.active_local_elements_end();

  for (const auto comp_index : make_range(_dim))
  {
    const auto var_num = momentum_system.variable_number(_u->name());

    for (auto it = begin; it != end; ++it)
    {
      const Elem * elem = *it;
      const auto dof_index = elem->dof_number(momentum_system.number(), var_num, 0);
      _HbyA[elem->id()](comp_index) = (*HbyA)(dof_index);
      _Ainv[elem->id()](comp_index) = (*Ainv)(dof_index);
    }
  }

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

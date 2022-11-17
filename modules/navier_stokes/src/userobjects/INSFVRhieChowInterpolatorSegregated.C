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
  exec_enum.addAvailableFlags(EXEC_NONLINEAR);
  exec_enum = {EXEC_NONLINEAR};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  params.addParam<NonlinearSystemName>(
      "momentum_system", "momentum_system", "The nonlinear system for the momentum equation");
  return params;
}

INSFVRhieChowInterpolatorSegregated::INSFVRhieChowInterpolatorSegregated(
    const InputParameters & params)
  : RhieChowInterpolatorBase(params),
    _HbyA(_moose_mesh, _sub_ids, "HbyA"),
    _Ainv(_moose_mesh, _sub_ids, "Ainv")
{
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
INSFVRhieChowInterpolatorSegregated::getVelocity(const FaceInfo & /*fi*/,
                                                 const Moose::StateArg & /*time*/,
                                                 THREAD_ID /*tid*/,
                                                 Moose::FV::InterpMethod /*m*/) const
{
  _console << "Something will come here eventually" << std::endl;
  return ADRealVectorValue(0.0);
}

void
INSFVRhieChowInterpolatorSegregated::computeHbyA()
{
  NonlinearSystemBase & sys = _fe_problem.getNonlinearSystemBase(
      _fe_problem.nlSysNum(getParam<NonlinearSystemName>("momentum_system")));

  NonlinearImplicitSystem & momentum_system = dynamic_cast<NonlinearImplicitSystem &>(sys.system());

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

  // We compute the right hand side with a zero vector for starters
  _fe_problem.computeResidualSys(momentum_system, *working_vector.get(), *HbyA.get());

  sys.setSolution(*solution);

  // _console << " Matrix initialized " << pmat->initialized() << std::endl;
  // _console << " Matrix closed " << pmat->closed() << std::endl;

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

  // HbyA_petsc->scale(-1.0);
  // pmat->get_diagonal(*Ainv_petsc);
  // pmat->vector_mult(*working_vector_petsc, *solution);
  // HbyA_petsc->add(*working_vector_petsc);

  // working_vector_petsc->pointwise_mult(*Ainv_petsc, *solution);
  // HbyA_petsc->add(-1.0, *working_vector_petsc);

  // *working_vector_petsc = 1.0;
  // VecPointwiseDivide(Ainv_petsc->vec(), working_vector_petsc->vec(), Ainv_petsc->vec());
  // HbyA_petsc->pointwise_mult(*HbyA_petsc, *Ainv_petsc);

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

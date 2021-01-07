#pragma once

#include "SolveObject.h"

#include "FormFunction.h"
#include "ExecFlagEnum.h"
#include <petsctao.h>
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

class FormFunction;

class OptimizeSolve : public SolveObject
{
public:
  static InputParameters validParams();
  OptimizeSolve(Executioner * ex);

  virtual bool solve() override;

  const FormFunction & getFormFunction() const { return *_form_function; }

protected:
  /// Bounds routine
  virtual PetscErrorCode variableBounds(Tao tao);

  /// Objective routine
  virtual Real objectiveFunction();

  /// Gradient routine
  virtual void gradientFunction(libMesh::PetscVector<Number> & gradient);

  /// Hessian routine
  virtual void hessianFunction(libMesh::PetscMatrix<Number> & hessian);

  /// Communicator used for operations
  const libMesh::Parallel::Communicator _my_comm;

  /// List of execute flags for when to solve the system
  const ExecFlagEnum & _solve_on;

  /// Form function defining objective, gradient, and hessian
  FormFunction * _form_function = nullptr;

  /// Tao optimization object
  Tao _tao;

private:
  /// Here is where we call tao and solve
  PetscErrorCode taoSolve();

  /// output optimization iteration solve data
  static PetscErrorCode monitor(Tao tao, void * ctx);

  ///@{
  /// Function wrappers for tao
  static PetscErrorCode objectiveFunctionWrapper(Tao tao, Vec x, Real * objective, void * ctx);
  static PetscErrorCode gradientFunctionWrapper(Tao tao, Vec x, Vec gradient, void * ctx);
  static PetscErrorCode hessianFunctionWrapper(Tao tao, Vec x, Mat hessian, Mat pc, void * ctx);
  static PetscErrorCode variableBoundsWrapper(Tao /*tao*/, Vec xl, Vec xu, void * ctx);
  ///@}

  // fixme lynn these have weird names because Petsc already has macros for TAONTR etc.
  /// Enum of tao solver types
  const enum class TaoSolverEnum {
    NEWTON_TRUST_REGION,
    BUNDLE_RISK_MIN,
    ORTHANT_QUASI_NEWTON,
    QUASI_NEWTON,
    CONJUGATE_GRADIENT,
    NELDER_MEAD,
    BOUNDED_QUASI_NEWTON,
    BOUNDED_CONJUGATE_GRADIENT,
    BOUNDED_QUASI_NEWTON_LINE_SEARCH,
    BOUNDED_NEWTON_LINE_SEARCH,
    BOUNDED_NEWTON_TRUST_REGION,
    GRADIENT_PROJECTION_CONJUGATE_GRADIENT,
  } _tao_solver_enum;

  /// Number of parameters being optimized
  dof_id_type _ndof;

  /// Parameters (solution)
  std::unique_ptr<libMesh::PetscVector<Number>> _parameters;

  /// Hessian (matrix)
  libMesh::PetscMatrix<Number> _hessian;
};

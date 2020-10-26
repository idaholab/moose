#pragma once

#include "SolveObject.h"

#include "FormFunction.h"
#include "ExecFlagEnum.h"
#include <petsctao.h>

class OptimizeSolve;

class OptimizeSolve : public SolveObject
{
public:
  static InputParameters validParams();
  OptimizeSolve(Executioner * ex);

  virtual bool solve() override;

  const FormFunction & getFormFunction() const { return *_form_function; }

protected:
  /// Objective routine
  virtual Real objectiveFunction(const libMesh::PetscVector<Number> & x);

  /// Gradient routine
  virtual void gradientFunction(const libMesh::PetscVector<Number> & x,
                                libMesh::PetscVector<Number> & gradient);

  /// Hessian routine
  virtual void hessianFunction(const libMesh::PetscVector<Number> & x,
                               libMesh::PetscMatrix<Number> & hessian);

  /// List of execute flags for when to solve the system
  const ExecFlagEnum & _solve_on;

  /// Form function defining objective, gradient, and hessian
  FormFunction * _form_function = nullptr;

  /// Tao optimization object
  Tao _tao;

private:
  /// output optimization iteration solve data
  static PetscErrorCode monitor(Tao tao, void * ctx);

  ///@{
  /// Function wrappers for tao
  ///
  static PetscErrorCode objectiveFunctionWrapper(Tao tao, Vec x, Real * objective, void * ctx);
  static PetscErrorCode gradientFunctionWrapper(Tao tao, Vec x, Vec gradient, void * ctx);
  static PetscErrorCode hessianFunctionWrapper(Tao tao, Vec x, Mat hessian, Mat pc, void * ctx);
  ///@}

  // fixme lynn these have weird names because Petsc already has macros for TAONTR etc.
  /// Enum of tao solver types
  const enum class TaoSolverEnum {
    NEWTON_TRUST_REGION, ///< NEWTON_TRUST_REGION
    CONJUGATE_GRADIENT,  ///< CONJUGATE_GRADIENT
    NELDER_MEAD,         ///< NELDER_MEAD
  } _tao_solver_enum;
};

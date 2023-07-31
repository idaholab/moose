//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolveObject.h"

#include "ExecFlagEnum.h"
#include <petsctao.h>
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

class OptimizationReporterBase;

/**
 * solveObject to interface with Petsc Tao
 */
class OptimizeSolve : public SolveObject
{
public:
  static InputParameters validParams();
  OptimizeSolve(Executioner & ex);

  virtual bool solve() override;

  const OptimizationReporterBase & getOptimizationReporter() const { return *_obj_function; }

  /**
   * Record tao TaoGetSolutionStatus data for output by a reporter
   * @param tot_iters total solves per iteration
   * @param gnorm gradient norm per iteration
   * @param obj_iters number of objective solves per iteration
   * @param cnorm infeasibility norm per iteration
   * @param grad_iters gradient solves per iteration
   * @param xdiff step length per iteration
   * @param hess_iters Hessian solves per iteration
   * @param f objective value per iteration
   * @param tot_solves total solves per iteration
   */
  void getTaoSolutionStatus(std::vector<int> & tot_iters,
                            std::vector<double> & gnorm,
                            std::vector<int> & obj_iters,
                            std::vector<double> & cnorm,
                            std::vector<int> & grad_iters,
                            std::vector<double> & xdiff,
                            std::vector<int> & hess_iters,
                            std::vector<double> & f,
                            std::vector<int> & tot_solves) const;

protected:
  /// Bounds routine
  virtual PetscErrorCode variableBounds(Tao tao);

  /// Objective routine
  virtual Real objectiveFunction();

  /// Gradient routine
  virtual void gradientFunction(libMesh::PetscVector<Number> & gradient);

  /// Hessian application routine
  virtual PetscErrorCode applyHessian(libMesh::PetscVector<Number> & s,
                                      libMesh::PetscVector<Number> & Hs);

  /// Communicator used for operations
  const libMesh::Parallel::Communicator _my_comm;

  /// List of execute flags for when to solve the system
  const ExecFlagEnum & _solve_on;

  /// objective function defining objective, gradient, and hessian
  OptimizationReporterBase * _obj_function = nullptr;

  /// Tao optimization object
  Tao _tao;

private:
  /// control optimization executioner output
  bool _verbose;

  ///@{
  /// count individual solves for output
  int _obj_iterate = 0;
  int _grad_iterate = 0;
  int _hess_iterate = 0;
  ///@}
  /// total solves per iteration
  std::vector<int> _total_iterate_vec;
  /// number of objective solves per iteration
  std::vector<int> _obj_iterate_vec;
  /// gradient solves per iteration
  std::vector<int> _grad_iterate_vec;
  /// Hessian solves per iteration
  std::vector<int> _hess_iterate_vec;
  /// total solves per iteration
  std::vector<int> _function_solve_vec;
  /// objective value per iteration
  std::vector<double> _f_vec;
  /// gradient norm per iteration
  std::vector<double> _gnorm_vec;
  /// infeasibility norm per iteration
  std::vector<double> _cnorm_vec;
  /// step length per iteration
  std::vector<double> _xdiff_vec;

  /// Here is where we call tao and solve
  PetscErrorCode taoSolve();

  /// output optimization iteration solve data
  void setTaoSolutionStatus(double f, int its, double gnorm, double cnorm, double xdiff);

  ///@{
  /// Function wrappers for tao
  static PetscErrorCode objectiveFunctionWrapper(Tao tao, Vec x, Real * objective, void * ctx);
  static PetscErrorCode hessianFunctionWrapper(Tao tao, Vec x, Mat hessian, Mat pc, void * ctx);
  static PetscErrorCode applyHessianWrapper(Mat H, Vec s, Vec Hs);
  static PetscErrorCode
  objectiveAndGradientFunctionWrapper(Tao tao, Vec x, Real * objective, Vec gradient, void * ctx);
  static PetscErrorCode variableBoundsWrapper(Tao /*tao*/, Vec xl, Vec xu, void * ctx);
  static PetscErrorCode monitor(Tao tao, void * ctx);
  ///@}

  /// Enum of tao solver types
  const enum class TaoSolverEnum {
    NEWTON_TRUST_REGION,
    BOUNDED_NEWTON_TRUST_REGION,
    BOUNDED_CONJUGATE_GRADIENT,
    NEWTON_LINE_SEARCH,
    BOUNDED_NEWTON_LINE_SEARCH,
    NEWTON_TRUST_LINE,
    BOUNDED_NEWTON_TRUST_LINE,
    QUASI_NEWTON,
    BOUNDED_QUASI_NEWTON,
    NELDER_MEAD,
    BOUNDED_QUASI_NEWTON_LINE_SEARCH,
    ORTHANT_QUASI_NEWTON,
    GRADIENT_PROJECTION_CONJUGATE_GRADIENT,
    BUNDLE_RISK_MIN
  } _tao_solver_enum;

  /// Number of parameters being optimized
  dof_id_type _ndof;

  /// Parameters (solution)
  std::unique_ptr<libMesh::PetscVector<Number>> _parameters;

  /// Hessian (matrix) - usually a matrix-free representation
  Mat _hessian;
};

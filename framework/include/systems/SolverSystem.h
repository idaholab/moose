//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SystemBase.h"
#include "MooseTypes.h"

#include "libmesh/system.h"

#include <string>

class SubProblem;
class FEProblemBase;

class SolverSystem : public SystemBase
{
public:
  SolverSystem(SubProblem & subproblem,
               FEProblemBase & fe_problem,
               const std::string & name,
               Moose::VarKindType var_kind);
  virtual ~SolverSystem();

  virtual void preInit() override;
  virtual void restoreSolutions() override final;

  void serializeSolution();

  /**
   * Quit the current solve as soon as possible.
   */
  virtual void stopSolve(const ExecFlagType & exec_flag) = 0;

  /**
   * Set the solution to a given vector.
   * @param soln The vector which should be treated as the solution.
   */
  void setSolution(const NumericVector<Number> & soln);

  /**
   * Set the side on which the preconditioner is applied to.
   * @param pcs The required preconditioning side
   */
  void setPCSide(MooseEnum pcs);

  /**
   * Get the current preconditioner side.
   */
  Moose::PCSideType getPCSide() { return _pc_side; }

  /**
   * Set the norm in which the linear convergence will be measured.
   * @param kspnorm The required norm
   */
  void setMooseKSPNormType(MooseEnum kspnorm);

  /**
   * Get the norm in which the linear convergence is measured.
   */
  Moose::MooseKSPNormType getMooseKSPNormType() { return _ksp_norm; }

  virtual const NumericVector<Number> * const & currentSolution() const override final;

  virtual void compute(ExecFlagType type) override;

protected:
  void checkInvalidSolution();

  virtual NumericVector<Number> & solutionInternal() const override final;

  /**
   * Whether a system matrix is formed from coloring. This influences things like when to compute
   * time derivatives
   */
  virtual bool matrixFromColoring() const { return false; }

  /// solution vector from solver
  const NumericVector<Number> * _current_solution;

  /// Preconditioning side
  Moose::PCSideType _pc_side;
  /// KSP norm type
  Moose::MooseKSPNormType _ksp_norm;

  /// Boolean to see if solution is invalid
  bool _solution_is_invalid;
};

inline const NumericVector<Number> * const &
SolverSystem::currentSolution() const
{
  return _current_solution;
}

inline NumericVector<Number> &
SolverSystem::solutionInternal() const
{
  return *system().solution;
}

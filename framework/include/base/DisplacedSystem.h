//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DISPLACEDSYSTEM_H
#define DISPLACEDSYSTEM_H

#include "SystemBase.h"

#include "libmesh/transient_system.h"
#include "libmesh/explicit_system.h"

// Forward declarations
class DisplacedProblem;

class DisplacedSystem : public SystemBase
{
public:
  DisplacedSystem(DisplacedProblem & problem,
                  SystemBase & undisplaced_system,
                  const std::string & name,
                  Moose::VarKindType var_kind);
  virtual ~DisplacedSystem();

  virtual void init() override;

  virtual bool hasResidualVector(Moose::KernelType type) const override
  {
    return _undisplaced_system.hasResidualVector(type);
  }

  virtual NumericVector<Number> & residualVector(Moose::KernelType type) override
  {
    return _undisplaced_system.residualVector(type);
  }

  virtual NumericVector<Number> & getVector(const std::string & name) override;

  virtual NumericVector<Number> & serializedSolution() override
  {
    return _undisplaced_system.serializedSolution();
  }

  virtual const NumericVector<Number> *& currentSolution() override
  {
    return _undisplaced_system.currentSolution();
  }

  virtual NumericVector<Number> & solution() override { return _undisplaced_system.solution(); }

  virtual NumericVector<Number> & solutionUDot() override
  {
    return _undisplaced_system.solutionUDot();
  }
  virtual Number & duDotDu() override { return _undisplaced_system.duDotDu(); }

  /**
   * Return the residual copy from the NonlinearSystem
   * @return Residual copy
   */
  virtual NumericVector<Number> & residualCopy() override
  {
    return _undisplaced_system.residualCopy();
  }
  virtual NumericVector<Number> & residualGhosted() override
  {
    return _undisplaced_system.residualGhosted();
  }

  virtual void augmentSendList(std::vector<dof_id_type> & send_list) override
  {
    _undisplaced_system.augmentSendList(send_list);
  }

  /**
   * This is an empty function since the displaced system doesn't have a matrix!
   * All sparsity pattern modification will be taken care of by the undisplaced system directly
   */
  virtual void augmentSparsity(SparsityPattern::Graph & /*sparsity*/,
                               std::vector<dof_id_type> & /*n_nz*/,
                               std::vector<dof_id_type> & /*n_oz*/) override
  {
  }

  /**
   * Adds this variable to the list of variables to be zeroed during each residual evaluation.
   * @param var_name The name of the variable to be zeroed.
   */
  virtual void addVariableToZeroOnResidual(std::string var_name) override
  {
    _undisplaced_system.addVariableToZeroOnResidual(var_name);
  }

  /**
   * Adds this variable to the list of variables to be zeroed during each jacobian evaluation.
   * @param var_name The name of the variable to be zeroed.
   */
  virtual void addVariableToZeroOnJacobian(std::string var_name) override
  {
    _undisplaced_system.addVariableToZeroOnJacobian(var_name);
  }

  /**
   * Zero out the solution for the list of variables passed in.
   */
  virtual void zeroVariables(std::vector<std::string> & vars_to_be_zeroed) override
  {
    _undisplaced_system.zeroVariables(vars_to_be_zeroed);
  }

  virtual NumericVector<Number> & solutionOld() override { return *_sys.old_local_solution; }

  virtual NumericVector<Number> & solutionOlder() override { return *_sys.older_local_solution; }

  virtual NumericVector<Number> * solutionPreviousNewton() override { return NULL; }

  virtual TransientExplicitSystem & sys() { return _sys; }

  virtual System & system() override { return _sys; }
  virtual const System & system() const override { return _sys; }

protected:
  SystemBase & _undisplaced_system;
  TransientExplicitSystem & _sys;
};

#endif /* DISPLACEDSYSTEM_H */

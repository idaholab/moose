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

// Forward declarations
class DisplacedProblem;
namespace libMesh
{
class ExplicitSystem;
template <typename>
class TransientSystem;
typedef TransientSystem<ExplicitSystem> TransientExplicitSystem;
}

using libMesh::TransientExplicitSystem;

class DisplacedSystem : public SystemBase
{
public:
  DisplacedSystem(DisplacedSystem &&) = delete;
  DisplacedSystem & operator=(DisplacedSystem &&) = delete;

  DisplacedSystem(DisplacedProblem & problem,
                  FEProblemBase & fe_problem,
                  SystemBase & undisplaced_system,
                  const std::string & name,
                  Moose::VarKindType var_kind);
  virtual ~DisplacedSystem();

  virtual NumericVector<Number> & getVector(TagID tag_id) override
  {
    return _undisplaced_system.getVector(tag_id);
  }
  virtual const NumericVector<Number> & getVector(TagID tag_id) const override
  {
    return _undisplaced_system.getVector(tag_id);
  }

  virtual TagID residualVectorTag() const override
  {
    return _undisplaced_system.residualVectorTag();
  }

  virtual TagID systemMatrixTag() const override { return _undisplaced_system.systemMatrixTag(); }

  virtual TagID timeVectorTag() const override { return _undisplaced_system.timeVectorTag(); }

  virtual TagID nonTimeVectorTag() const override { return _undisplaced_system.nonTimeVectorTag(); }

  virtual std::set<TagID> defaultVectorTags() const override
  {
    return _undisplaced_system.defaultVectorTags();
  }
  virtual std::set<TagID> defaultMatrixTags() const override
  {
    return _undisplaced_system.defaultMatrixTags();
  }

  virtual void associateVectorToTag(NumericVector<Number> & vec, TagID tag) override
  {
    _undisplaced_system.associateVectorToTag(vec, tag);
  }

  virtual void disassociateVectorFromTag(NumericVector<Number> & vec, TagID tag) override
  {
    _undisplaced_system.disassociateVectorFromTag(vec, tag);
  }

  virtual void disassociateVectorFromTag(TagID tag) override
  {
    _undisplaced_system.disassociateVectorFromTag(tag);
  }

  virtual void disassociateDefaultVectorTags() override
  {
    _undisplaced_system.disassociateDefaultVectorTags();
  }

  virtual void associateMatrixToTag(libMesh::SparseMatrix<Number> & matrix, TagID tag) override
  {
    _undisplaced_system.associateMatrixToTag(matrix, tag);
  }

  virtual void disassociateMatrixFromTag(libMesh::SparseMatrix<Number> & matrix, TagID tag) override
  {
    _undisplaced_system.disassociateMatrixFromTag(matrix, tag);
  }

  virtual void disassociateMatrixFromTag(TagID tag) override
  {
    _undisplaced_system.disassociateMatrixFromTag(tag);
  }

  virtual void disassociateDefaultMatrixTags() override
  {
    _undisplaced_system.disassociateDefaultMatrixTags();
  }

  virtual NumericVector<Number> & getVector(const std::string & name) override;
  virtual const NumericVector<Number> & getVector(const std::string & name) const override;

  virtual NumericVector<Number> & serializedSolution() override
  {
    return _undisplaced_system.serializedSolution();
  }

  const NumericVector<Number> * const & currentSolution() const override
  {
    return _undisplaced_system.currentSolution();
  }

  NumericVector<Number> * solutionPreviousNewton() override { return nullptr; }
  const NumericVector<Number> * solutionPreviousNewton() const override { return nullptr; }

  NumericVector<Number> * solutionUDot() override { return _undisplaced_system.solutionUDot(); }
  NumericVector<Number> * solutionUDotDot() override
  {
    return _undisplaced_system.solutionUDotDot();
  }
  NumericVector<Number> * solutionUDotOld() override
  {
    return _undisplaced_system.solutionUDotOld();
  }
  NumericVector<Number> * solutionUDotDotOld() override
  {
    return _undisplaced_system.solutionUDotDotOld();
  }
  const NumericVector<Number> * solutionUDot() const override
  {
    return _undisplaced_system.solutionUDot();
  }
  const NumericVector<Number> * solutionUDotDot() const override
  {
    return _undisplaced_system.solutionUDotDot();
  }
  const NumericVector<Number> * solutionUDotOld() const override
  {
    return _undisplaced_system.solutionUDotOld();
  }
  const NumericVector<Number> * solutionUDotDotOld() const override
  {
    return _undisplaced_system.solutionUDotDotOld();
  }

  virtual std::vector<Number> & duDotDus() override { return _undisplaced_system.duDotDus(); }
  virtual Number & duDotDotDu() override { return _undisplaced_system.duDotDotDu(); }
  virtual const Number & duDotDu(unsigned int var_num = 0) const override;
  virtual const Number & duDotDotDu() const override { return _undisplaced_system.duDotDotDu(); }

  virtual void addDotVectors() override { _undisplaced_system.addDotVectors(); }

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
  virtual void augmentSparsity(libMesh::SparsityPattern::Graph & /*sparsity*/,
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

  virtual bool hasVector(TagID tag_id) const override
  {
    return _undisplaced_system.hasVector(tag_id);
  }

  virtual bool hasMatrix(TagID tag_id) const override
  {
    return _undisplaced_system.hasMatrix(tag_id);
  }

  virtual void initSolutionState() override;
  virtual NumericVector<Number> & solutionState(
      const unsigned int state,
      Moose::SolutionIterationType iteration_type = Moose::SolutionIterationType::Time) override;
  virtual const NumericVector<Number> &
  solutionState(const unsigned int state,
                Moose::SolutionIterationType iteration_type =
                    Moose::SolutionIterationType::Time) const override;
  virtual void needSolutionState(
      const unsigned int state,
      Moose::SolutionIterationType iteration_type = Moose::SolutionIterationType::Time) override;
  virtual bool hasSolutionState(const unsigned int state,
                                Moose::SolutionIterationType iteration_type =
                                    Moose::SolutionIterationType::Time) const override;

  virtual libMesh::SparseMatrix<Number> & getMatrix(TagID tag) override
  {
    return _undisplaced_system.getMatrix(tag);
  }
  virtual const libMesh::SparseMatrix<Number> & getMatrix(TagID tag) const override
  {
    return _undisplaced_system.getMatrix(tag);
  }

  virtual TransientExplicitSystem & sys() { return _sys; }

  virtual System & system() override;
  virtual const System & system() const override;

  virtual void compute(ExecFlagType) override {}

protected:
  NumericVector<Number> & solutionInternal() const override
  {
    return _undisplaced_system.solution();
  }

  SystemBase & _undisplaced_system;
  TransientExplicitSystem & _sys;
};

inline void
DisplacedSystem::initSolutionState()
{
  _undisplaced_system.initSolutionState();
}

inline NumericVector<Number> &
DisplacedSystem::solutionState(const unsigned int state,
                               const Moose::SolutionIterationType iteration_type)
{
  return _undisplaced_system.solutionState(state, iteration_type);
}

inline const NumericVector<Number> &
DisplacedSystem::solutionState(const unsigned int state,
                               const Moose::SolutionIterationType iteration_type) const
{
  return _undisplaced_system.solutionState(state, iteration_type);
}

inline void
DisplacedSystem::needSolutionState(const unsigned int state,
                                   const Moose::SolutionIterationType iteration_type)
{
  _undisplaced_system.needSolutionState(state, iteration_type);
}

inline bool
DisplacedSystem::hasSolutionState(const unsigned int state,
                                  const Moose::SolutionIterationType iteration_type) const
{
  return _undisplaced_system.hasSolutionState(state, iteration_type);
}

inline const Number &
DisplacedSystem::duDotDu(const unsigned int var_num) const
{
  return _undisplaced_system.duDotDu(var_num);
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodeElemConstraint.h"

class DisplacedProblem;
class FEProblemBase;

/**
 * A EqualValueEmbeddedConstraint forces the value of a variable to be the same
 * on overlapping portion of two blocks
 */
class EqualValueEmbeddedConstraint : public NodeElemConstraint
{
public:
  static InputParameters validParams();

  EqualValueEmbeddedConstraint(const InputParameters & parameters);

  virtual void timestepSetup() override {}
  virtual void jacobianSetup() override {}
  virtual void residualEnd() override {}

  virtual bool addCouplingEntriesToJacobian() override { return true; }

  bool shouldApply() override;

  /**
   * Prepare the residual contribution of the current constraint required to enforce it
   * based on the specified formulation.
   */
  void reinitConstraint();

protected:
  virtual void prepareSecondaryToPrimaryMap() override;
  virtual Real computeQpSecondaryValue() override;
  virtual ADReal computeQpResidual(Moose::ConstraintType type) override;

  MooseSharedPointer<DisplacedProblem> _displaced_problem;
  FEProblem & _fe_problem;

  /// Formulations, currently only supports KINEMATIC and PENALTY
  const enum class Formulation { KINEMATIC, PENALTY } _formulation;
  /// Penalty parameter used in constraint enforcement for kinematic and penalty formulations
  const ADReal _penalty;
  /// copy of the residual before the constraint is applied
  NumericVector<Number> & _residual_copy;
  /// constraint force needed to enforce the constraint
  ADReal _constraint_residual;
};

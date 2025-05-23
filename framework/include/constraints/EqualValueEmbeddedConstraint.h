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
#include "GenericNodeElemConstraint.h"

class DisplacedProblem;
class FEProblemBase;

/**
 * A EqualValueEmbeddedConstraint forces the value of a variable to be the same
 * on overlapping portion of two blocks
 */
template <bool is_ad>
class EqualValueEmbeddedConstraintTempl : public GenericNodeElemConstraint<is_ad>
{
public:
  static InputParameters validParams();

  EqualValueEmbeddedConstraintTempl(const InputParameters & parameters);

  virtual void timestepSetup() override {}
  virtual void jacobianSetup() override {}
  virtual void residualEnd() override {}

  virtual bool addCouplingEntriesToJacobian() override { return true; }

  bool shouldApply() override;

  /**
   * Prepare the residual contribution of the current constraint required to enforce it
   * based on the specified formulation.
   */
  virtual void reinitConstraint();

protected:
  virtual void prepareSecondaryToPrimaryMap() override;
  virtual Real computeQpSecondaryValue() override;
  virtual GenericReal<is_ad> computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                        unsigned int jvar) override;

  MooseSharedPointer<DisplacedProblem> _displaced_problem;
  FEProblem & _fe_problem;

  /// Formulations, currently only supports KINEMATIC and PENALTY
  const enum class Formulation { KINEMATIC, PENALTY } _formulation;
  /// Penalty parameter used in constraint enforcement for kinematic and penalty formulations
  const Real _penalty;
  /// copy of the residual before the constraint is applied
  NumericVector<Number> & _residual_copy;
  /// constraint force needed to enforce the constraint
  GenericReal<is_ad> _constraint_residual;

  usingGenericNodeElemConstraint;
};

typedef EqualValueEmbeddedConstraintTempl<false> EqualValueEmbeddedConstraint;
typedef EqualValueEmbeddedConstraintTempl<true> ADEqualValueEmbeddedConstraint;

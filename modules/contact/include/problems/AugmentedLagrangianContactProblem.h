//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ReferenceResidualProblem.h"
#include "FEProblem.h"
#include "NodeFaceConstraint.h"
#include "MechanicalContactConstraint.h"
#include "AugmentedLagrangianContactProblemInterface.h"

/**
 * Class to manage nested solution for augmented Lagrange contact.
 * The AugmentedLagrangianContactProblem manages the nested solution procedure,
 * repeating the solution until convergence has been achieved, checking for convergence, and
 * updating the Lagrangian multipliers.
 */
template <class T>
class AugmentedLagrangianContactProblemTempl : public T,
                                               public AugmentedLagrangianContactProblemInterface
{
public:
  static InputParameters validParams();

  AugmentedLagrangianContactProblemTempl(const InputParameters & params);
  virtual ~AugmentedLagrangianContactProblemTempl() {}

  virtual void timestepSetup() override;
  virtual void addDefaultNonlinearConvergence(const InputParameters & params) override;
  virtual bool onlyAllowDefaultNonlinearConvergence() const override { return true; }

protected:
  using AugmentedLagrangianContactProblemInterface::_lagrangian_iteration_number;
  using AugmentedLagrangianContactProblemInterface::_maximum_number_lagrangian_iterations;
};

typedef AugmentedLagrangianContactProblemTempl<ReferenceResidualProblem>
    AugmentedLagrangianContactProblem;
typedef AugmentedLagrangianContactProblemTempl<FEProblem> AugmentedLagrangianContactFEProblem;

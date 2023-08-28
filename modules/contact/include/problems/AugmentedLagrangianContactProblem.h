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

class AugmentedLagrangianContactProblemInterface
{
public:
  static InputParameters validParams();
  AugmentedLagrangianContactProblemInterface(const InputParameters & params);
  virtual const unsigned int & getLagrangianIterationNumber() const
  {
    return _lagrangian_iteration_number;
  }

protected:
  /// maximum mumber of augmented lagrange iterations
  const unsigned int _maximum_number_lagrangian_iterations;

  /// current augmented lagrange iteration number
  unsigned int _lagrangian_iteration_number;
};

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

  virtual MooseNonlinearConvergenceReason
  checkNonlinearConvergence(std::string & msg,
                            const PetscInt it,
                            const Real xnorm,
                            const Real snorm,
                            const Real fnorm,
                            const Real rtol,
                            const Real divtol,
                            const Real stol,
                            const Real abstol,
                            const PetscInt nfuncs,
                            const PetscInt max_funcs,
                            const Real ref_resid,
                            const Real div_threshold) override;

protected:
  using AugmentedLagrangianContactProblemInterface::_lagrangian_iteration_number;
  using AugmentedLagrangianContactProblemInterface::_maximum_number_lagrangian_iterations;
  using FEProblem::_console;
  using FEProblem::currentNonlinearSystem;
  using FEProblem::geomSearchData;
  using FEProblem::getDisplacedProblem;
  using FEProblem::theWarehouse;
};

typedef AugmentedLagrangianContactProblemTempl<ReferenceResidualProblem>
    AugmentedLagrangianContactProblem;
typedef AugmentedLagrangianContactProblemTempl<FEProblem> AugmentedLagrangianContactFEProblem;

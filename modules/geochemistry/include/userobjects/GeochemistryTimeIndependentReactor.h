//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "EquilibriumGeochemicalSolver.h"
#include "GeochemistryReactorBase.h"

/**
 * Class that controls the time independent (and spatially independent) geochemistry reactions
 */
class GeochemistryTimeIndependentReactor : public GeochemistryReactorBase
{
public:
  static InputParameters validParams();
  GeochemistryTimeIndependentReactor(const InputParameters & parameters);
  virtual void initialize() override;

  virtual void finalize() override;

  virtual void execute() override;

  virtual const EquilibriumGeochemicalSystem &
  getEquilibriumGeochemicalSystem(const Point & point) const override;
  virtual const std::stringstream & getSolverOutput(const Point & point) const override;
  virtual unsigned getSolverIterations(const Point & point) const override;
  virtual Real getSolverResidual(const Point & point) const override;
  virtual const EquilibriumGeochemicalSystem &
  getEquilibriumGeochemicalSystem(unsigned node_id) const override;

protected:
  const Real _temperature;
  /// The equilibrium geochemical system that holds all the molalities, activities, etc
  EquilibriumGeochemicalSystem _egs;
  /// The solver
  EquilibriumGeochemicalSolver _solver;
};

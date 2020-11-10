//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeochemicalSolver.h"
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

  virtual void initialSetup() override;
  virtual void execute() override;

  virtual const GeochemicalSystem & getGeochemicalSystem(dof_id_type node_id) const override;
  virtual const std::stringstream & getSolverOutput(dof_id_type node_id) const override;
  virtual unsigned getSolverIterations(dof_id_type node_id) const override;
  virtual Real getSolverResidual(dof_id_type node_id) const override;
  virtual const DenseVector<Real> & getMoleAdditions(dof_id_type node_id) const override;
  virtual Real getMolesDumped(dof_id_type node_id, const std::string & species) const override;

protected:
  const Real _temperature;
  /// The equilibrium geochemical system that holds all the molalities, activities, etc
  GeochemicalSystem _egs;
  /// The solver
  GeochemicalSolver _solver;
  /// mole additions: these are always zero but are used in getMoleAdditions and the solve
  DenseVector<Real> _mole_additions;
};

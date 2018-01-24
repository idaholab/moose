//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FULLSOLVEMULTIAPP_H
#define FULLSOLVEMULTIAPP_H

#include "MultiApp.h"

// Forward declarations
class FullSolveMultiApp;
class Executioner;

template <>
InputParameters validParams<FullSolveMultiApp>();

/**
 * This type of MultiApp will completely solve itself the first time it is asked to take a step.
 *
 * Each "step" after that it will do nothing.
 */
class FullSolveMultiApp : public MultiApp
{
public:
  FullSolveMultiApp(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) override;

  virtual void advanceStep() override {}

  virtual bool isSolved() const override { return _solved; }

private:
  std::vector<Executioner *> _executioners;

  /// Whether or not this MultiApp has already been solved.
  bool _solved;
};

#endif // FULLSOLVEMULTIAPP_H

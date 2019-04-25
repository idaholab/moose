//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiApp.h"

// Forward declarations
class FullSolveMultiApp;
class Executioner;

template <>
InputParameters validParams<FullSolveMultiApp>();

/**
 * This type of MultiApp will do a full solve when it is asked to take a step.
 */
class FullSolveMultiApp : public MultiApp
{
public:
  FullSolveMultiApp(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) override;

  virtual void postExecute() override
  {
    // executioner postExecute has been called and we do not need to call it again
  }

  virtual void backup() override;
  virtual void restore() override;

private:
  std::vector<Executioner *> _executioners;
};


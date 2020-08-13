//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Transient.h"

// Forward Declarations
class PODTransient;
class TimeStepper;
class FEProblemBase;

class PODTransient : public Transient
{
public:
  static InputParameters validParams();

  PODTransient(const InputParameters & parameters);

  virtual void execute() override;

protected:

  PerfID _post_snapshot_timer;

};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeStepper.h"
#include "ExternalPETScProblem.h"

class ExternalPetscTimeStepper : public TimeStepper
{
public:
  static InputParameters validParams();

  ExternalPetscTimeStepper(const InputParameters & parameters);

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

private:
  ExternalPETScProblem & _external_petsc_problem;
};

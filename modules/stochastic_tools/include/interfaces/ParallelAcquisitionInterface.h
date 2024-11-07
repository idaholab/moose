//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "FEProblemBase.h"
#include "ParallelAcquisitionFunctionBase.h"

class ParallelAcquisitionInterface
{
public:
  static InputParameters validParams();

  ParallelAcquisitionInterface(const InputParameters & parameters);

protected:
  /// Lookup a ParallelAcquisitionFunction object by name and return pointer
  ParallelAcquisitionFunctionBase * getParallelAcquisitionFunctionByName(const UserObjectName & name) const;

private:
  /// Reference to FEProblemBase instance
  FEProblemBase & _parallelacquisition_feproblem;
};

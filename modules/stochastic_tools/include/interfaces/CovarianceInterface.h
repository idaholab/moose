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
#include "CovarianceFunctionBase.h"

class CovarianceInterface
{
public:
  static InputParameters validParams();

  CovarianceInterface(const InputParameters & parameters);

protected:
  /// Lookup a CovarianceFunction object by name and return pointer
  CovarianceFunctionBase * getCovarianceFunctionByName(const UserObjectName & name) const;

private:
  /// Reference to FEProblemBase instance
  FEProblemBase & _covar_feproblem;
};

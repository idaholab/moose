//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "FEProblemBase.h"

class CovarianceFunctionBaseTorched;

class CovarianceInterfaceTorched
{
public:
  static InputParameters validParams();

  CovarianceInterfaceTorched(const InputParameters & parameters);

protected:
  /// Lookup a CovarianceFunction object by name and return pointer
  CovarianceFunctionBaseTorched * getCovarianceFunctionByName(const UserObjectName & name) const;

private:
  /// Reference to FEProblemBase instance
  FEProblemBase & _covar_feproblem;
};

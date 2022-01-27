//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarExecutorInterface.h"
#include "FEProblem.h"
#include "MortarData.h"

MortarExecutorInterface::MortarExecutorInterface(FEProblemBase & fe_problem)
  : _mortar_data(fe_problem.mortarData())
{
  _mortar_data.notifyWhenMortarSetup(this);
}

MortarExecutorInterface::MortarExecutorInterface(MortarExecutorInterface && other)
  : _mortar_data(other._mortar_data)
{
  _mortar_data.dontNotifyWhenMortarSetup(&other);
  _mortar_data.notifyWhenMortarSetup(this);
}

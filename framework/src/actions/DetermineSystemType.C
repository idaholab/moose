//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DetermineSystemType.h"
#include "MooseApp.h"

registerMooseAction("MooseApp", DetermineSystemType, "determine_system_type");

template <>
InputParameters
validParams<DetermineSystemType>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.mooseObjectSyntaxVisibility(false);
  return params;
}

DetermineSystemType::DetermineSystemType(InputParameters parameters) : MooseObjectAction(parameters)
{
}

void
DetermineSystemType::act()
{
  /**
   * Determine whether the Executioner is derived from EigenExecutionerBase and
   * set a flag on MooseApp that can be used during problem construction.
   */
  if (_moose_object_pars.isParamValid("_eigen") && _moose_object_pars.get<bool>("_eigen"))
    _app.useNonlinear() = false;

  if (_moose_object_pars.isParamValid("_use_eigen_value") &&
      _moose_object_pars.get<bool>("_use_eigen_value"))
    _app.useEigenvalue() = true;
}

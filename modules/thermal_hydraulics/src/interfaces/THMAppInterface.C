//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMAppInterface.h"
#include "InputParameters.h"
#include "ThermalHydraulicsApp.h"

InputParameters
THMAppInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

THMAppInterface::THMAppInterface(const InputParameters & params)
  : _thm_app(initializeThermalHydraulicsAppReference(params))
{
}

ThermalHydraulicsApp &
THMAppInterface::initializeThermalHydraulicsAppReference(const InputParameters & params)
{
  MooseApp & moose_app =
      *params.getCheckedPointerParam<MooseApp *>("_moose_app", "In Action constructor");
  auto thm_app = dynamic_cast<ThermalHydraulicsApp *>(&moose_app);
  if (thm_app)
    return *thm_app;
  else
    mooseError("THMAppInterface may only be used from a ThermalHydraulicsApp.");
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunctionRadiativeBC.h"

registerKokkosResidualObject("HeatTransferApp", KokkosFunctionRadiativeBC);

InputParameters
KokkosFunctionRadiativeBC::validParams()
{
  InputParameters params = KokkosRadiativeHeatFluxBCBase::validParams();
  params.addClassDescription("Boundary condition for radiative heat exchange where the emissivity "
                             "function is supplied by a Function.");
  params.addRequiredParam<FunctionName>(
      "emissivity_function", "Function describing emissivity for radiative boundary condition");
  return params;
}

KokkosFunctionRadiativeBC::KokkosFunctionRadiativeBC(const InputParameters & parameters)
  : KokkosRadiativeHeatFluxBCBase(parameters), _emissivity(getKokkosFunction("emissivity_function"))
{
}

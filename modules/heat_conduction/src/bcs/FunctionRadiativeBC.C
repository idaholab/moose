//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionRadiativeBC.h"
#include "MathUtils.h"

registerMooseObject("HeatConductionApp", FunctionRadiativeBC);

InputParameters
FunctionRadiativeBC::validParams()
{
  InputParameters params = RadiativeHeatFluxBCBase::validParams();
  params.addClassDescription("Boundary condition for radiative heat exchange where the emissivity "
                             "function is supplied by a Function.");
  params.addRequiredParam<FunctionName>(
      "emissivity_function", "Function describing emissivity for radiative boundary condition");
  return params;
}

FunctionRadiativeBC::FunctionRadiativeBC(const InputParameters & parameters)
  : RadiativeHeatFluxBCBase(parameters), _emissivity(getFunction("emissivity_function"))
{
}

Real
FunctionRadiativeBC::coefficient() const
{
  return _emissivity.value(_t, _q_point[_qp]);
}

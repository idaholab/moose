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
registerMooseObject("HeatConductionApp", ADFunctionRadiativeBC);

template <bool is_ad>
InputParameters
FunctionRadiativeBCTempl<is_ad>::validParams()
{
  InputParameters params = RadiativeHeatFluxBCBaseTempl<is_ad>::validParams();
  params.addClassDescription("Boundary condition for radiative heat exchange where the emissivity "
                             "function is supplied by a Function.");
  params.addRequiredParam<FunctionName>(
      "emissivity_function", "Function describing emissivity for radiative boundary condition");
  return params;
}

template <bool is_ad>
FunctionRadiativeBCTempl<is_ad>::FunctionRadiativeBCTempl(const InputParameters & parameters)
  : RadiativeHeatFluxBCBaseTempl<is_ad>(parameters), _emissivity(getFunction("emissivity_function"))
{
}

template <bool is_ad>
GenericReal<is_ad>
FunctionRadiativeBCTempl<is_ad>::coefficient() const
{
  return _emissivity.value(_t, _q_point[_qp]);
}

template class FunctionRadiativeBCTempl<false>;
template class FunctionRadiativeBCTempl<true>;

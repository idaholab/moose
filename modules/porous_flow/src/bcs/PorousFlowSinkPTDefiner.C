/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowSinkPTDefiner.h"

template <>
InputParameters
validParams<PorousFlowSinkPTDefiner>()
{
  InputParameters params = validParams<PorousFlowSink>();
  return params;
}

PorousFlowSinkPTDefiner::PorousFlowSinkPTDefiner(const InputParameters & parameters)
  : PorousFlowSink(parameters),
    _pp(_involves_fluid ? &getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_nodal")
                        : nullptr),
    _dpp_dvar(_involves_fluid
                  ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                        "dPorousFlow_porepressure_nodal_dvar")
                  : nullptr),
    _temp(!_involves_fluid ? &getMaterialProperty<Real>("PorousFlow_temperature_nodal") : nullptr),
    _dtemp_dvar(!_involves_fluid
                    ? &getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")
                    : nullptr)
{
  if (_involves_fluid && (_pp == nullptr || _dpp_dvar == nullptr))
    mooseError("PorousFlowSink: There is no porepressure Material");
  if (!_involves_fluid && (_temp == nullptr || _dtemp_dvar == nullptr))
    mooseError("PorousFlowSink: There is no temperature Material");
}

Real
PorousFlowSinkPTDefiner::ptVar() const
{
  if (_involves_fluid)
    return (*_pp)[_i][_ph];
  return (*_temp)[_i];
}

Real
PorousFlowSinkPTDefiner::dptVar(unsigned pvar) const
{
  if (_involves_fluid)
    return (*_dpp_dvar)[_i][_ph][pvar];
  return (*_dtemp_dvar)[_i][pvar];
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MapMultiplyCoupledVars.h"

registerMooseObject("MooseTestApp", MapMultiplyCoupledVars);

InputParameters
MapMultiplyCoupledVars::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("v", "The first coupled var");
  params.addRequiredCoupledVar("w", "The second coupled var");
  params.addRequiredParam<std::map<std::string, Real>>("coupled_var_multipliers",
                                                       "A map from variable names to multipliers");
  params.addRequiredParam<std::map<std::string, std::string>>(
      "dummy_string_to_string_map",
      "A dummy parameter just to make sure that we can parse a string to string map");
  return params;
}

MapMultiplyCoupledVars::MapMultiplyCoupledVars(const InputParameters & parameters)
  : ADKernel(parameters),
    _v(adCoupledValue("v")),
    _w(adCoupledValue("w")),
    _coupled_map(getParam<std::map<std::string, Real>>("coupled_var_multipliers")),
    _dummy_string_to_string_map(
        getParam<std::map<std::string, std::string>>("dummy_string_to_string_map"))
{
  if (_coupled_map.size() != 2)
    paramError("coupled_var_multipliers", "We need exactly 2 multipliers");

  {
    auto it = _coupled_map.find(getParam<std::vector<VariableName>>("v").front());
    if (it == _coupled_map.end())
      paramError("coupled_var_multipliers",
                 "no multiplier entry for ",
                 getParam<std::vector<VariableName>>("v").front());
    _v_multiplier = it->second;
  }

  {
    auto it = _coupled_map.find(getParam<std::vector<VariableName>>("w").front());
    if (it == _coupled_map.end())
      paramError("coupled_var_multipliers",
                 "no multiplier entry for ",
                 getParam<std::vector<VariableName>>("w").front());
    _w_multiplier = it->second;
  }
}

ADReal
MapMultiplyCoupledVars::computeQpResidual()
{
  return _test[_i][_qp] * (_v_multiplier * _v[_qp] + _w_multiplier * _w[_qp]);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowTemperature.h"

registerMooseObject("PorousFlowApp", PorousFlowTemperature);

InputParameters
PorousFlowTemperature::validParams()
{
  InputParameters params = PorousFlowMaterial::validParams();
  params.addCoupledVar("temperature",
                       293.0,
                       "Fluid temperature variable.  Note, the default is suitable if your "
                       "simulation is using Kelvin units, but probably not for Celsius");
  params.addPrivateParam<std::string>("pf_material_type", "temperature");
  params.addClassDescription("Material to provide temperature at the quadpoints or nodes and "
                             "derivatives of it with respect to the PorousFlow variables");
  return params;
}

PorousFlowTemperature::PorousFlowTemperature(const InputParameters & parameters)
  : DerivativeMaterialInterface<PorousFlowMaterial>(parameters),

    _num_pf_vars(_dictator.numVariables()),
    _temperature_var(_nodal_material ? coupledDofValues("temperature")
                                     : coupledValue("temperature")),
    _grad_temperature_var(_nodal_material ? nullptr : &coupledGradient("temperature")),
    _temperature_is_PF(_dictator.isPorousFlowVariable(coupled("temperature"))),
    _t_var_num(_temperature_is_PF ? _dictator.porousFlowVariableNum(coupled("temperature")) : 0),

    _temperature(_nodal_material ? declareProperty<Real>("PorousFlow_temperature_nodal")
                                 : declareProperty<Real>("PorousFlow_temperature_qp")),
    _dtemperature_dvar(
        _nodal_material ? declareProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")
                        : declareProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar")),
    _grad_temperature(_nodal_material
                          ? nullptr
                          : &declareProperty<RealGradient>("PorousFlow_grad_temperature_qp")),
    _dgrad_temperature_dgradv(_nodal_material ? nullptr
                                              : &declareProperty<std::vector<Real>>(
                                                    "dPorousFlow_grad_temperature_qp_dgradvar")),
    _dgrad_temperature_dv(_nodal_material ? nullptr
                                          : &declareProperty<std::vector<RealGradient>>(
                                                "dPorousFlow_grad_temperature_qp_dvar"))
{
}

void
PorousFlowTemperature::initQpStatefulProperties()
{
  computeQpProperties();
}

void
PorousFlowTemperature::computeQpProperties()
{
  _temperature[_qp] = _temperature_var[_qp];
  _dtemperature_dvar[_qp].assign(_num_pf_vars, 0.0);
  if (_temperature_is_PF)
    // _temperature is a PorousFlow variable
    _dtemperature_dvar[_qp][_t_var_num] = 1.0;

  if (!_nodal_material)
  {
    (*_grad_temperature)[_qp] = (*_grad_temperature_var)[_qp];
    (*_dgrad_temperature_dgradv)[_qp].assign(_num_pf_vars, 0.0);
    (*_dgrad_temperature_dv)[_qp].assign(_num_pf_vars, RealGradient());
    if (_temperature_is_PF)
      (*_dgrad_temperature_dgradv)[_qp][_t_var_num] = 1.0;
  }
}

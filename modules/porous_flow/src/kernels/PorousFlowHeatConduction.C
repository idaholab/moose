/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowHeatConduction.h"

template <>
InputParameters
validParams<PorousFlowHeatConduction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("Heat conduction in the Porous Flow module");
  return params;
}

PorousFlowHeatConduction::PorousFlowHeatConduction(const InputParameters & parameters)
  : Kernel(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _la(getMaterialProperty<RealTensorValue>("PorousFlow_thermal_conductivity_qp")),
    _dla_dvar(getMaterialProperty<std::vector<RealTensorValue>>(
        "dPorousFlow_thermal_conductivity_qp_dvar")),
    _grad_t(getMaterialProperty<RealGradient>("PorousFlow_grad_temperature_qp")),
    _dgrad_t_dvar(
        getMaterialProperty<std::vector<RealGradient>>("dPorousFlow_grad_temperature_qp_dvar")),
    _dgrad_t_dgradvar(
        getMaterialProperty<std::vector<Real>>("dPorousFlow_grad_temperature_qp_dgradvar"))
{
}

Real
PorousFlowHeatConduction::computeQpResidual()
{
  return _grad_test[_i][_qp] * (_la[_qp] * _grad_t[_qp]);
}

Real
PorousFlowHeatConduction::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
PorousFlowHeatConduction::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;

  /// The PorousFlow variable index corresponding to the variable number jvar
  const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);

  return _grad_test[_i][_qp] *
         ((_dla_dvar[_qp][pvar] * _grad_t[_qp] + _la[_qp] * _dgrad_t_dvar[_qp][pvar]) *
              _phi[_j][_qp] +
          _la[_qp] * _dgrad_t_dgradvar[_qp][pvar] * _grad_phi[_j][_qp]);
}

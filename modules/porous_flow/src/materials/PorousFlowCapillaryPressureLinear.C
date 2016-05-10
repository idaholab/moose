/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowCapillaryPressureLinear.h"

template<>
InputParameters validParams<PorousFlowCapillaryPressureLinear>()
{
  InputParameters params = validParams<PorousFlowCapillaryPressureBase>();
  params.addRequiredParam<Real>("pc_max",  "Maximum capillary pressure (Pa)");
  params.addClassDescription("This Material provides a linear capillary pressure");
  return params;
}

PorousFlowCapillaryPressureLinear::PorousFlowCapillaryPressureLinear(const InputParameters & parameters) :
  PorousFlowCapillaryPressureBase(parameters),
  _pc_max(getParam<Real>("pc_max"))
{
}

void
PorousFlowCapillaryPressureLinear::computeQpProperties()
{
  /// Capillary pressure and derivatives wrt phase saturation at the nodes
  _capillary_pressure_nodal[_qp] = _pc_max * (1.0 - _saturation_nodal[_qp][_phase_num]);
  _dcapillary_pressure_nodal_ds[_qp] = - _pc_max;
  _d2capillary_pressure_nodal_ds2[_qp] = 0.0;

  /// Capillary pressure and derivatives wrt phase saturation at the qps
  _capillary_pressure_qp[_qp] = _pc_max * (1.0 - _saturation_qp[_qp][_phase_num]);
  _dcapillary_pressure_qp_ds[_qp] = - _pc_max;
  _d2capillary_pressure_qp_ds2[_qp] = 0.0;
}

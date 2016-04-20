/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialCapillaryPressureLinear.h"

template<>
InputParameters validParams<PorousFlowMaterialCapillaryPressureLinear>()
{
  InputParameters params = validParams<PorousFlowMaterialCapillaryPressureBase>();
  params.addRequiredParam<Real>("pc_max",  "Maximum capillary pressure (Pa)");
  params.addClassDescription("This Material provides a linear relative permeability");
  return params;
}

PorousFlowMaterialCapillaryPressureLinear::PorousFlowMaterialCapillaryPressureLinear(const InputParameters & parameters) :
  PorousFlowMaterialCapillaryPressureBase(parameters),
  _pc_max(getParam<Real>("pc_max"))
{
}

void
PorousFlowMaterialCapillaryPressureLinear::computeQpProperties()
{
  _capillary_pressure[_qp] = _pc_max * (1.0 - _saturation[_qp][_phase_num]);
  _dcapillary_pressure_ds[_qp] = - _pc_max;
}

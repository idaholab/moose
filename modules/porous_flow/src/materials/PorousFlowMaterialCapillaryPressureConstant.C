/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialCapillaryPressureConstant.h"

template<>
InputParameters validParams<PorousFlowMaterialCapillaryPressureConstant>()
{
  InputParameters params = validParams<PorousFlowMaterialCapillaryPressureBase>();
  params.addRequiredParam<Real>("capillary_pressure", "The capillary pressure (Pa)");
  params.addClassDescription("This Material provides a constantant capillary pressure (Pa)");
  return params;
}

PorousFlowMaterialCapillaryPressureConstant::PorousFlowMaterialCapillaryPressureConstant(const InputParameters & parameters) :
    PorousFlowMaterialCapillaryPressureBase(parameters),
    _pc(getParam<Real>("capillary_pressure"))
{
}

void
PorousFlowMaterialCapillaryPressureConstant::computeQpProperties()
{
  _capillary_pressure[_qp] = _pc;
  _dcapillary_pressure_ds[_qp] = 0.0;
}

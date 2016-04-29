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
  /// Capillary pressure and derivatives wrt phase saturation at the nodes
  _capillary_pressure_nodal[_qp] = _pc;
  _dcapillary_pressure_nodal_ds[_qp] = 0.0;
  _d2capillary_pressure_nodal_ds2[_qp] = 0.0;

  /// Capillary pressure and derivatives wrt phase saturation at the qps
  _capillary_pressure_qp[_qp] = _pc;
  _dcapillary_pressure_qp_ds[_qp] = 0.0;
  _d2capillary_pressure_qp_ds2[_qp] = 0.0;
}

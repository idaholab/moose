/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowCapillaryPressureConstant.h"

template<>
InputParameters validParams<PorousFlowCapillaryPressureConstant>()
{
  InputParameters params = validParams<PorousFlowCapillaryPressureBase>();
  params.addRequiredParam<Real>("capillary_pressure", "The capillary pressure (Pa)");
  params.addClassDescription("This Material provides a constantant capillary pressure (Pa)");
  return params;
}

PorousFlowCapillaryPressureConstant::PorousFlowCapillaryPressureConstant(const InputParameters & parameters) :
    PorousFlowCapillaryPressureBase(parameters),
    _pc(getParam<Real>("capillary_pressure"))
{
}

void
PorousFlowCapillaryPressureConstant::computeQpProperties()
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

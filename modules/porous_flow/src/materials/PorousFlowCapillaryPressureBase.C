/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowCapillaryPressureBase.h"

template<>
InputParameters validParams<PorousFlowCapillaryPressureBase>()
{
  InputParameters params = validParams<PorousFlowMaterialBase>();
  params.addClassDescription("Base class for PorousFlow capillary pressure materials");
  return params;
}

PorousFlowCapillaryPressureBase::PorousFlowCapillaryPressureBase(const InputParameters & parameters) :
    PorousFlowMaterialBase(parameters),
    _saturation_variable_name(_dictator.saturationVariableNameDummy()),
    _saturation_nodal(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_nodal")),
    _saturation_qp(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_qp")),
    _capillary_pressure_nodal(declareProperty<Real>("PorousFlow_capillary_pressure_nodal" + _phase)),
    _dcapillary_pressure_nodal_ds(declarePropertyDerivative<Real>("PorousFlow_capillary_pressure_nodal" + _phase, _saturation_variable_name)),
    _d2capillary_pressure_nodal_ds2(declarePropertyDerivative<Real>("PorousFlow_capillary_pressure_nodal" + _phase, _saturation_variable_name, _saturation_variable_name)),
    _capillary_pressure_qp(declareProperty<Real>("PorousFlow_capillary_pressure_qp" + _phase)),
    _dcapillary_pressure_qp_ds(declarePropertyDerivative<Real>("PorousFlow_capillary_pressure_qp" + _phase, _saturation_variable_name)),
    _d2capillary_pressure_qp_ds2(declarePropertyDerivative<Real>("PorousFlow_capillary_pressure_qp" + _phase, _saturation_variable_name, _saturation_variable_name))
{
}

void
PorousFlowCapillaryPressureBase::computeQpProperties()
{
  mooseError("computeQpProperties() must be overriden in materials derived from PorousFlowCapillaryPressureBase");
}

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowViscosityConst.h"

template <>
InputParameters
validParams<PorousFlowViscosityConst>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addRequiredParam<Real>("viscosity",
                                "The viscosity, which is assumed constant for this material");
  params.addClassDescription("This Material calculates the viscosity assuming it is constant");
  return params;
}

PorousFlowViscosityConst::PorousFlowViscosityConst(const InputParameters & parameters)
  : PorousFlowFluidPropertiesBase(parameters),
    _input_viscosity(getParam<Real>("viscosity")),
    _viscosity(_nodal_material ? declareProperty<Real>("PorousFlow_viscosity_nodal" + _phase)
                               : declareProperty<Real>("PorousFlow_viscosity_qp" + _phase))
{
}

void
PorousFlowViscosityConst::computeQpProperties()
{
  _viscosity[_qp] = _input_viscosity;
}

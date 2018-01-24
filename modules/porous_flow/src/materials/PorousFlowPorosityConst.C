/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPorosityConst.h"

template <>
InputParameters
validParams<PorousFlowPorosityConst>()
{
  InputParameters params = validParams<PorousFlowPorosityBase>();
  params.addRequiredCoupledVar("porosity", "The porosity (assumed constant for this material)");
  params.addClassDescription("This Material calculates the porosity assuming it is constant");
  return params;
}

PorousFlowPorosityConst::PorousFlowPorosityConst(const InputParameters & parameters)
  : PorousFlowPorosityBase(parameters), _input_porosity(coupledValue("porosity"))
{
}

void
PorousFlowPorosityConst::initQpStatefulProperties()
{
  _porosity[_qp] = _input_porosity[_qp];
}

void
PorousFlowPorosityConst::computeQpProperties()
{
  initQpStatefulProperties();

  // The derivatives are zero for all time
  _dporosity_dvar[_qp].assign(_num_var, 0.0);
  _dporosity_dgradvar[_qp].assign(_num_var, RealGradient());
}

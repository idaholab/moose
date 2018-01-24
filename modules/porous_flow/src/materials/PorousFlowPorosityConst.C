//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

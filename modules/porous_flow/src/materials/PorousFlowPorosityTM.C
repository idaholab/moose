//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityTM.h"

template <>
InputParameters
validParams<PorousFlowPorosityTM>()
{
  InputParameters params = validParams<PorousFlowPorosity>();
  params.set<bool>("mechanical") = true;
  params.set<bool>("thermal") = true;
  params.set<Real>("biot_coefficient") = 1.0;
  return params;
}

PorousFlowPorosityTM::PorousFlowPorosityTM(const InputParameters & parameters)
  : PorousFlowPorosity(parameters)
{
  mooseDeprecated("PorousFlowPorosityTM is deprecated.  In your input file please replace\n    "
                  "type = PorousFlowPorosityTM\nwith\n    type = PorousFlowPorosity\n    "
                  "mechanical = true\n    thermal = true\n    biot_coefficient = 1.0");
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityHM.h"

template <>
InputParameters
validParams<PorousFlowPorosityHM>()
{
  InputParameters params = validParams<PorousFlowPorosity>();
  params.set<bool>("mechanical") = true;
  params.set<bool>("fluid") = true;
  return params;
}

PorousFlowPorosityHM::PorousFlowPorosityHM(const InputParameters & parameters)
  : PorousFlowPorosity(parameters)
{
  mooseDeprecated("PorousFlowPorosityHM is deprecated.  In your input file please replace\n    "
                  "type = PorousFlowPorosityHM\nwith\n    type = PorousFlowPorosity\n    "
                  "mechanical = true\n    fluid = true");
}

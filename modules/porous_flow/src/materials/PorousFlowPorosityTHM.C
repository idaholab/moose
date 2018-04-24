//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityTHM.h"

registerMooseObjectDeprecated("PorousFlowApp", PorousFlowPorosityTHM, "09/01/2018 00:00");

template <>
InputParameters
validParams<PorousFlowPorosityTHM>()
{
  InputParameters params = validParams<PorousFlowPorosity>();
  params.set<bool>("mechanical") = true;
  params.set<bool>("fluid") = true;
  params.set<bool>("thermal") = true;
  return params;
}

PorousFlowPorosityTHM::PorousFlowPorosityTHM(const InputParameters & parameters)
  : PorousFlowPorosity(parameters)
{
  mooseDeprecated("PorousFlowPorosityTHM is deprecated.  In your input file please replace\n    "
                  "type = PorousFlowPorosityTHM\nwith\n    type = PorousFlowPorosity\n    "
                  "mechanical = true\n    fluid = true\n    thermal = true");
}

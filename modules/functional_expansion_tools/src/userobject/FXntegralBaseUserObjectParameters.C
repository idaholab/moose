//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FXIntegralBaseUserObject.h"

template <>
InputParameters
validParams<FXIntegralBaseUserObjectParameters>()
{
  InputParameters params = validParams<MutableCoefficientsInterface>();

  params.addClassDescription(
      "This UserObject interacts with a MooseApp through functional expansions.");

  params.addRequiredParam<FunctionName>("function",
                                        "The name of the FunctionSeries \"Function\" object with "
                                        "which to generate this functional expansion.");

  params.addParam<bool>(
      "keep_history", false, "Keep the expansion coefficients from previous solves");

  params.addParam<bool>("print_state", false, "Print the state of the zeroth instance each solve");

  return params;
}

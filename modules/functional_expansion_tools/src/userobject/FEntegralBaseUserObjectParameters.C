/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FEIntegralBaseUserObject.h"

template <>
InputParameters
validParams<FEIntegralBaseUserObjectParameters>()
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

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

#include "NodalVariablePostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

template<>
InputParameters validParams<NodalVariablePostprocessor>()
{
  InputParameters params = validParams<NodalPostprocessor>();
  params.addCoupledVar("variable", "The name of the variable that this postprocessor operates on");
  return params;
}

NodalVariablePostprocessor::NodalVariablePostprocessor(const std::string & name, InputParameters parameters) :
    NodalPostprocessor(name, parameters),
    MooseVariableInterface(parameters, true),
    _u(coupledValue("variable"))
{
}

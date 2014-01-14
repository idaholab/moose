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

#include "Receiver.h"

template<>
InputParameters validParams<Receiver>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addParam<Real>("default", "The default value");

  return params;
}

Receiver::Receiver(const std::string & name, InputParameters params) :
    GeneralPostprocessor(name, params),
    _my_value(getPostprocessorValueByName(name))
{
}

Real
Receiver::getValue()
{
  // Return the stored value (references stored value in getPostprocessorData)
  return _my_value;
}

void
Receiver::initialSetup()
{
  if (isParamValid("default"))
    _fe_problem.getPostprocessorValue(_pp_name) = getParam<Real>("default");
}

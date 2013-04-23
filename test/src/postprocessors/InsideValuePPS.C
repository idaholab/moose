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

#include "InsideValuePPS.h"
#include "InsideUserObject.h"

template<>
InputParameters validParams<InsideValuePPS>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<UserObjectName>("user_object", "The name of the user object");

  return params;
}

InsideValuePPS::InsideValuePPS(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _uo(getUserObject<InsideUserObject>("user_object")),
    _value(0.)
{
}

InsideValuePPS::~InsideValuePPS()
{
}

void
InsideValuePPS::initialize()
{
  _value = 0;
}

void
InsideValuePPS::execute()
{
  _value = _uo.getValue();
}

Real
InsideValuePPS::getValue()
{
  return _value;
}

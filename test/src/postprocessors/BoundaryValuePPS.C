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

// moose_test includes
#include "BoundaryUserObject.h"
#include "BoundaryValuePPS.h"


template<>
InputParameters validParams<BoundaryValuePPS>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<UserObjectName>("user_object", "The name of the user object");

  return params;
}

BoundaryValuePPS::BoundaryValuePPS(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _uo(getUserObject<BoundaryUserObject>("user_object")),
    _value(0.)
{
}

BoundaryValuePPS::~BoundaryValuePPS()
{
}

void
BoundaryValuePPS::initialize()
{
  _value = 0;
}

void
BoundaryValuePPS::execute()
{
  _value = _uo.getValue();
}

Real
BoundaryValuePPS::getValue()
{
  return _value;
}

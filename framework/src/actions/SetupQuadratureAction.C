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

#include "SetupQuadratureAction.h"
#include "Conversion.h"
#include "MProblem.h"

template<>
InputParameters validParams<SetupQuadratureAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<std::string>("type", "GAUSS", "Type of the quadrature rule");
  params.addParam<std::string>("order", "AUTO", "Order of the quadrature");

  return params;
}

SetupQuadratureAction::SetupQuadratureAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters),
    _type(Moose::stringToEnum<QuadratureType>(getParam<std::string>("type"))),
    _order(Moose::stringToEnum<Order>(getParam<std::string>("order")))
{
}

SetupQuadratureAction::~SetupQuadratureAction()
{
}

void
SetupQuadratureAction::act()
{
  _problem->createQRules(_type, _order);
}

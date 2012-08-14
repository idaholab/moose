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
#include "FEProblem.h"
#include "MooseEnum.h"

template<>
InputParameters validParams<SetupQuadratureAction>()
{
  MooseEnum types("CLOUGH, CONICAL, GAUSS, GRID, MONOMIAL, SIMPSON, TRAP", "GAUSS");
  MooseEnum order("AUTO, CONSTANT, FIRST, SECOND, THIRD, FOURTH, FIFTH, SIXTH, SEVENTH, EIGHTH, NINTH, TENTH, "
                  "ELEVENTH, TWELFTH, THIRTEENTH, FOURTEENTH, FIFTEENTH, SIXTEENTH, SEVENTEENTH, EIGHTTEENTH, NINTEENTH, TWENTIETH", "AUTO");

  InputParameters params = validParams<Action>();

  params.addParam<MooseEnum>("type", types, "Type of the quadrature rule");
  params.addParam<MooseEnum>("order", order, "Order of the quadrature");

  return params;
}

SetupQuadratureAction::SetupQuadratureAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters),
    _type(Moose::stringToEnum<QuadratureType>(getParam<MooseEnum>("type"))),
    _order(Moose::stringToEnum<Order>(getParam<MooseEnum>("order")))
{
}

SetupQuadratureAction::~SetupQuadratureAction()
{
}

void
SetupQuadratureAction::act()
{
  if (_problem != NULL)
    _problem->createQRules(_type, _order);
}

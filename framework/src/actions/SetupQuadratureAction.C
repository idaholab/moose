//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupQuadratureAction.h"
#include "Conversion.h"
#include "FEProblem.h"
#include "MooseEnum.h"

registerMooseAction("MooseApp", SetupQuadratureAction, "setup_quadrature");

defineLegacyParams(SetupQuadratureAction);

InputParameters
SetupQuadratureAction::validParams()
{
  MooseEnum types("CLOUGH CONICAL GAUSS GRID MONOMIAL SIMPSON TRAP GAUSS_LOBATTO", "GAUSS");
  MooseEnum order("AUTO CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH TENTH "
                  "ELEVENTH TWELFTH THIRTEENTH FOURTEENTH FIFTEENTH SIXTEENTH SEVENTEENTH "
                  "EIGHTTEENTH NINTEENTH TWENTIETH",
                  "AUTO");

  InputParameters params = Action::validParams();

  params.addParam<MooseEnum>("type", types, "Type of the quadrature rule");
  params.addParam<MooseEnum>("order", order, "Order of the quadrature");
  params.addParam<MooseEnum>("element_order", order, "Order of the quadrature for elements");
  params.addParam<MooseEnum>("side_order", order, "Order of the quadrature for sides");
  params.addParam<std::vector<SubdomainID>>("custom_blocks",
                                            std::vector<SubdomainID>{},
                                            "list of blocks to specify custom quadrature order");
  params.addParam<std::vector<std::string>>("custom_orders",
                                            std::vector<std::string>{},
                                            "list of quadrature orders (e.g. FIRST, SECOND, etc.)");

  return params;
}

SetupQuadratureAction::SetupQuadratureAction(InputParameters parameters)
  : Action(parameters),
    _type(Moose::stringToEnum<QuadratureType>(getParam<MooseEnum>("type"))),
    _order(Moose::stringToEnum<Order>(getParam<MooseEnum>("order"))),
    _element_order(Moose::stringToEnum<Order>(getParam<MooseEnum>("element_order"))),
    _side_order(Moose::stringToEnum<Order>(getParam<MooseEnum>("side_order"))),
    _custom_blocks(getParam<std::vector<SubdomainID>>("custom_blocks")),
    _custom_orders(getParam<std::vector<std::string>>("custom_orders"))
{
}

void
SetupQuadratureAction::act()
{
  if (_problem.get() == nullptr)
    return;

  // add default/global quadrature rules
  _problem->createQRules(_type, _order, _element_order, _side_order);

  // add custom block-specific quadrature rules
  for (unsigned int i = 0; i < _custom_blocks.size(); i++)
    _problem->createQRules(_type,
                           _order,
                           Moose::stringToEnum<Order>(_custom_orders[i]),
                           _side_order,
                           _custom_blocks[i]);
}

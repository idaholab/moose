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

template <>
InputParameters
validParams<SetupQuadratureAction>()
{
  MooseEnum types("CLOUGH CONICAL GAUSS GRID MONOMIAL SIMPSON TRAP GAUSS_LOBATTO", "GAUSS");
  MooseEnum order("AUTO CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH TENTH "
                  "ELEVENTH TWELFTH THIRTEENTH FOURTEENTH FIFTEENTH SIXTEENTH SEVENTEENTH "
                  "EIGHTTEENTH NINTEENTH TWENTIETH",
                  "AUTO");

  InputParameters params = validParams<Action>();

  params.addParam<MooseEnum>("type", types, "Type of the quadrature rule");
  params.addParam<MooseEnum>("order", order, "Order of the quadrature");
  params.addParam<MooseEnum>("element_order", order, "Order of the quadrature for elements");
  params.addParam<MooseEnum>("side_order", order, "Order of the quadrature for sides");
  params.addParam<std::vector<SubdomainID>>("custom_blocks",
                                            std::vector<SubdomainID>{},
                                            "list of blocks to specify custom quadrature order");
  params.addParam<std::vector<std::string>>("custom_element_orders",
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
    _blocks(getParam<std::vector<SubdomainID>>("custom_blocks")),
    _elem_orders(getParam<std::vector<std::string>>("custom_element_orders"))
{
}

void
SetupQuadratureAction::act()
{
  if (_problem.get() == nullptr)
    return;

  _problem->createQRules(_type, _order, _element_order, _side_order);

  if (_blocks.size() != _elem_orders.size())
    paramError("custom_element_orders",
               "number of orders (",
               _elem_orders.size(),
               ") doesn't match number of blocks (",
               _blocks.size(),
               ")");

  for (unsigned int i = 0; i < _blocks.size(); i++)
    _problem->createQRules(
        _type, _order, Moose::stringToEnum<Order>(_elem_orders[i]), _side_order, _blocks[i]);
}

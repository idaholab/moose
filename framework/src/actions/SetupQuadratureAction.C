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

InputParameters
SetupQuadratureAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Sets the quadrature type for the simulation.");
  params.addParam<MooseEnum>("type", getQuadratureTypesEnum(), "Type of the quadrature rule");
  params.addParam<MooseEnum>("order", getQuadratureOrderEnum(), "Order of the quadrature");
  params.addParam<MooseEnum>(
      "element_order", getQuadratureOrderEnum(), "Order of the quadrature for elements");
  params.addParam<MooseEnum>(
      "side_order", getQuadratureOrderEnum(), "Order of the quadrature for sides");
  params.addParam<std::vector<SubdomainID>>("custom_blocks",
                                            std::vector<SubdomainID>{},
                                            "list of blocks to specify custom quadrature order");
  params.addParam<MultiMooseEnum>(
      "custom_orders",
      getQuadratureOrdersMultiEnum(),
      "list of quadrature orders for the blocks specified in `custom_blocks`");
  params.addParam<bool>(
      "allow_negative_qweights", true, "Whether or not allow negative quadrature weights");

  return params;
}

SetupQuadratureAction::SetupQuadratureAction(const InputParameters & parameters)
  : Action(parameters),
    _type(Moose::stringToEnum<libMesh::QuadratureType>(getParam<MooseEnum>("type"))),
    _order(Moose::stringToEnum<Order>(getParam<MooseEnum>("order"))),
    _element_order(Moose::stringToEnum<Order>(getParam<MooseEnum>("element_order"))),
    _side_order(Moose::stringToEnum<Order>(getParam<MooseEnum>("side_order"))),
    _custom_block_orders(getParam<SubdomainID, MooseEnumItem>("custom_blocks", "custom_orders")),
    _allow_negative_qweights(getParam<bool>("allow_negative_qweights"))
{
}

void
SetupQuadratureAction::act()
{
  if (_problem.get() == nullptr)
    return;

  // add default/global quadrature rules
  _problem->createQRules(
      _type, _order, _element_order, _side_order, Moose::ANY_BLOCK_ID, _allow_negative_qweights);

  // add custom block-specific quadrature rules
  for (const auto & [block, order] : _custom_block_orders)
    _problem->createQRules(_type,
                           _order,
                           Moose::stringToEnum<Order>(order),
                           Moose::stringToEnum<Order>(order),
                           block,
                           _allow_negative_qweights);
}

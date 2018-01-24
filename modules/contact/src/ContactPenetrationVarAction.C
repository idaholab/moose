/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ContactPenetrationVarAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseApp.h"
#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<ContactPenetrationVarAction>()
{
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addParam<MooseEnum>("order", orders, "The finite element order: FIRST, SECOND, etc.");
  return params;
}

ContactPenetrationVarAction::ContactPenetrationVarAction(const InputParameters & params)
  : Action(params)
{
}

void
ContactPenetrationVarAction::act()
{
  if (!_problem->getDisplacedProblem())
    mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
               "Mesh block.");

  _problem->addAuxVariable("penetration",
                           FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                                  Utility::string_to_enum<FEFamily>("LAGRANGE")));
}

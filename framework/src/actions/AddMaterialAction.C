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

#include "AddMaterialAction.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AddMaterialAction>()
{
  return validParams<MooseObjectAction>();
}

AddMaterialAction::AddMaterialAction(InputParameters params) : MooseObjectAction(params) {}

void
AddMaterialAction::act()
{
  _problem->addMaterial(_type, _name, _moose_object_pars);
}

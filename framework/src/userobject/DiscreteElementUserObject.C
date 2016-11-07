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

#include "DiscreteElementUserObject.h"

template <>
InputParameters
validParams<DiscreteElementUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();

  // UOs of this type should not be executed by MOOSE, but only called directly by the user
  params.set<MultiMooseEnum>("execute_on") = "custom";
  params.suppressParameter<MultiMooseEnum>("execute_on");

  return params;
}

DiscreteElementUserObject::DiscreteElementUserObject(const InputParameters & parameters)
  : ElementUserObject(parameters)
{
}

void
DiscreteElementUserObject::initialize()
{
}

void
DiscreteElementUserObject::execute()
{
  mooseError("DiscreteElementUserObjects must be called explicitly from Materials");
}

void
DiscreteElementUserObject::finalize()
{
  mooseError("DiscreteElementUserObjects must be called explicitly from Materials");
}

void
DiscreteElementUserObject::threadJoin(const UserObject &)
{
  mooseError("DiscreteElementUserObjects must be called explicitly from Materials");
}

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

#include "MaterialUserObject.h"

template<>
InputParameters validParams<MaterialUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  return params;
}

MaterialUserObject::MaterialUserObject(const InputParameters & parameters) :
    ElementUserObject(parameters)
{
}

void
MaterialUserObject::execute()
{
  mooseError("MaterialUserObjects must be called explicitly from Materials");
}

void
MaterialUserObject::finalize()
{
  mooseError("MaterialUserObjects must be called explicitly from Materials");
}

void
MaterialUserObject::threadJoin(const UserObject &)
{
  mooseError("MaterialUserObjects must be called explicitly from Materials");
}

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

#include "GeneralUserObject.h"

template<>
InputParameters validParams<GeneralUserObject>()
{
  InputParameters params = validParams<UserObject>();
  return params;
}

GeneralUserObject::GeneralUserObject(const std::string & name, InputParameters parameters) :
    UserObject(name, parameters),
    MaterialPropertyInterface(parameters),
    TransientInterface(parameters, name, "general_user_objects"),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    PostprocessorInterface(parameters)
{}

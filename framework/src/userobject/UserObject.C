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

#include "UserObject.h"

template<>
InputParameters validParams<UserObject>()
{
  InputParameters params = validParams<MooseObject>();
  params.addPrivateParam<std::string>("built_by_action", "add_user_object");
  return params;
}

UserObject::UserObject(const std::string & name, InputParameters params) :
    MooseObject(name, params)
{
}

UserObject::~UserObject()
{
}

void
UserObject::load(std::ifstream & /*stream*/)
{
}

void
UserObject::store(std::ofstream & /*stream*/)
{
}

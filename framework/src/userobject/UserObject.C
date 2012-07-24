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

#include "SubProblem.h"

template<>
InputParameters validParams<UserObject>()
{
  InputParameters params = validParams<MooseObject>();
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addPrivateParam<std::string>("built_by_action", "add_user_object");
  return params;
}

UserObject::UserObject(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    _problem(*parameters.get<Problem *>("_problem")),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _coord_sys(_subproblem.coordSystem(_tid))
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

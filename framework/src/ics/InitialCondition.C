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

#include "InitialCondition.h"

template<>
InputParameters validParams<InitialCondition>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<std::string>("var_name", "The variable this InitialCondtion is supposed to provide values for.");

  params.addPrivateParam<std::string>("built_by_action", "add_ic");
  return params;
}

InitialCondition::InitialCondition(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    FunctionInterface(parameters),
    _var_name(getParam<std::string>("var_name"))
{
}

InitialCondition::~InitialCondition()
{
}


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

#include "Postprocessor.h"
#include "UserObject.h"

template <>
InputParameters
validParams<Postprocessor>()
{
  InputParameters params = validParams<UserObject>();
  params += validParams<OutputInterface>();

  params.addParamNamesToGroup("outputs", "Advanced");
  params.registerBase("Postprocessor");
  return params;
}

Postprocessor::Postprocessor(const InputParameters & parameters)
  : OutputInterface(parameters), _pp_name(parameters.get<std::string>("_object_name"))
{
}

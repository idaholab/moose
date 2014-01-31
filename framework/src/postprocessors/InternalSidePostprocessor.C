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

#include "InternalSidePostprocessor.h"

template<>
InputParameters validParams<InternalSidePostprocessor>()
{
  InputParameters params = validParams<InternalSideUserObject>();
  params += validParams<Postprocessor>();
  return params;
}

InternalSidePostprocessor::InternalSidePostprocessor(const std::string & name, InputParameters parameters) :
    InternalSideUserObject(name, parameters),
    Postprocessor(name, parameters)
{
}

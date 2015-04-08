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
#include "ExceptionSteady.h"

template<>
InputParameters validParams<ExceptionSteady>()
{
  return validParams<Steady>();
}

ExceptionSteady::ExceptionSteady(const std::string & name, InputParameters parameters) :
    Steady(name, parameters)
{
}

ExceptionSteady::~ExceptionSteady()
{
}

void
ExceptionSteady::execute()
{
  try
  {
    Steady::execute();
  }
  catch (MooseException & e)
  {
    Moose::err << "Caught exception " << e << std::endl;
  }
}

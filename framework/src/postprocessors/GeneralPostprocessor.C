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

#include "GeneralPostprocessor.h"

template<>
InputParameters validParams<GeneralPostprocessor>()
{
  InputParameters params = validParams<Postprocessor>();
  return params;
}

GeneralPostprocessor::GeneralPostprocessor(const std::string & name, InputParameters parameters) :
    Postprocessor(name, parameters),
    PostprocessorInterface(parameters)
{}

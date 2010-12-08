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

#include "EmptyPostprocessor.h"

#include "MooseSystem.h"

// libMesh includes
#include "parallel.h"

template<>
InputParameters validParams<EmptyPostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

EmptyPostprocessor::EmptyPostprocessor(const std::string & name, InputParameters parameters)
  :GeneralPostprocessor(name, parameters)
{}





/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
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
  InputParameters params = validParams<MooseObject>();
  return params;
}

EmptyPostprocessor::EmptyPostprocessor(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Postprocessor(name, moose_system, parameters)
{}





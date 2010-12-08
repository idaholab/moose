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

#include "PrintNumNodes.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<PrintNumNodes>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

PrintNumNodes::PrintNumNodes(const std::string & name, InputParameters parameters):
  GeneralPostprocessor(name, parameters)
{}

Real
PrintNumNodes::getValue()
{
  return _moose_system.getMesh()->n_nodes();
}

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

#include "PrintDOFs.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<PrintDOFs>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

PrintDOFs::PrintDOFs(const std::string & name, InputParameters parameters):
  GeneralPostprocessor(name, parameters)
{}

Real
PrintDOFs::getValue()
{
  return _moose_system.getEquationSystems()->n_dofs();
}

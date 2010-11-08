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

#include "PrintDT.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<PrintDT>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

PrintDT::PrintDT(const std::string & name, MooseSystem &moose_system, InputParameters parameters):
  GeneralPostprocessor(name, moose_system, parameters)
{}

Real
PrintDT::getValue()
{
  return _moose_system._dt;
}

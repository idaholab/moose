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
#include "FEProblem.h"

template<>
InputParameters validParams<PrintDT>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

PrintDT::PrintDT(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _feproblem(dynamic_cast<FEProblem &>(_subproblem))
{}

Real
PrintDT::getValue()
{
  return _feproblem.dt();
}

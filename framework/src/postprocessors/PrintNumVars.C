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

#include "PrintNumVars.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PrintNumVars>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  MooseEnum system_options("nonlinear, auxiliary", "nonlinear");
  params.addParam<MooseEnum>("system", system_options, "The system for which you want to print the number of variables.");

  return params;
}

PrintNumVars::PrintNumVars(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _system(getParam<MooseEnum>("system"))
{}

Real
PrintNumVars::getValue()
{
  switch(_system)
  {
    case 0:
      return _fe_problem.getNonlinearSystem().sys().n_vars();
    case 1:
      return _fe_problem.getAuxiliarySystem().sys().n_vars();
  }

  mooseError("Unknown system type!");
}

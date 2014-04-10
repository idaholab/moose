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

#include "ProblemRealParameter.h"
#include "FEProblem.h"

template<>
InputParameters validParams<ProblemRealParameter>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<std::string>("param_name", "Name of the parameter to be exposed");
  return params;
}

ProblemRealParameter::ProblemRealParameter(const std::string & name,
                                           InputParameters parameters) :
    GeneralPostprocessor(name, parameters)
{}

Real
ProblemRealParameter::getValue()
{
  return _fe_problem.getParam<Real>(getParam<std::string>("param_name"));
}

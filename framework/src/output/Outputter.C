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

#include "Outputter.h"
#include "Problem.h"

Outputter::Outputter(EquationSystems & es) :
    _es(es)
{
}

Outputter::~Outputter()
{
}

void
Outputter::setOutputVariables(std::vector<VariableName> output_variables)
{
  _output_variables.resize(output_variables.size());
  for (unsigned int i=0; i<output_variables.size(); ++i)
    _output_variables[i] = output_variables[i];
}

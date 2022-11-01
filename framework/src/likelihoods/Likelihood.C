//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Likelihood.h"
#include "MooseRandom.h"

InputParameters
Likelihood::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addRequiredParam<std::string>("type",
                                       "class/type name identifying the likelihood function");
  params.registerBase("Likelihood");
  params.registerSystemAttributeName("Likelihood");
  return params;
}

Likelihood::Likelihood(const InputParameters & parameters)
  : MooseObject(parameters), PerfGraphInterface(this)
{
}

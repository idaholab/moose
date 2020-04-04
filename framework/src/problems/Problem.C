//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Problem.h"
#include "Factory.h"
#include "Function.h"

InputParameters
Problem::validParams()
{
  InputParameters params = MooseObject::validParams();

  params.registerBase("Problem");
  return params;
}

Problem::Problem(const InputParameters & parameters)
  : MooseObject(parameters),
    PerfGraphInterface(this),
    _cli_option_found(false),
    _color_output(false),
    _termination_requested(false)
{
}

Problem::~Problem() {}

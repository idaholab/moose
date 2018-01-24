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

#include "Problem.h"
#include "Factory.h"
#include "Function.h"

template <>
InputParameters
validParams<Problem>()
{
  InputParameters params;
  params += validParams<MooseObject>();
  params.registerBase("Problem");
  return params;
}

Problem::Problem(const InputParameters & parameters)
  : MooseObject(parameters),
    _cli_option_found(false),
    _color_output(false),
    _termination_requested(false)
{
}

Problem::~Problem() {}

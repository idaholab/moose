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

#include "Distribution.h"
#include "MooseRandom.h"

template <>
InputParameters
validParams<Distribution>()
{
  InputParameters params = validParams<MooseObject>();
  params.registerBase("Distribution");
  return params;
}

Distribution::Distribution(const InputParameters & parameters) : MooseObject(parameters) {}

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

#include "GrowPP.h"

template<>
InputParameters validParams<GrowPP>()
{
InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

GrowPP::GrowPP(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _old_val(getPostprocessorValueOldByName(name))
{}

GrowPP::~GrowPP()
{
}

void
GrowPP::initialize()
{
}

void
GrowPP::execute()
{
}

Real
GrowPP::getValue()
{
  if (_t_step == 0)
    return 1;

  return _old_val + 1;
}

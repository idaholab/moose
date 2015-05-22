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
  params.addParam<bool>("use_older_value", false, "Use the older value of the postprocessor for this test");
  return params;
}

GrowPP::GrowPP(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _use_older(getParam<bool>("use_older_value")),
    _old_val(getPostprocessorValueOldByName(name)),
    _older_val(getPostprocessorValueOlderByName(name))
{
}

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

  if (_use_older)
    return _old_val + _older_val;
  else
    return _old_val + 1;
}

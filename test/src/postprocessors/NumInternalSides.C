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

#include "NumInternalSides.h"

template<>
InputParameters validParams<NumInternalSides>()
{
  InputParameters params = validParams<InternalSidePostprocessor>();
  return params;
}

NumInternalSides::NumInternalSides(const std::string & name, InputParameters parameters) :
    InternalSidePostprocessor(name, parameters),
    _count(0)
{
}

NumInternalSides::~NumInternalSides()
{
}

void
NumInternalSides::execute()
{
  _count++;
}

void
NumInternalSides::initialize()
{
  _count = 0;
}

void
NumInternalSides::finalize()
{
  gatherSum(_count);
}


PostprocessorValue
NumInternalSides::getValue()
{
  return _count;
}

void
NumInternalSides::threadJoin(const UserObject & uo)
{
  const NumInternalSides & obj = static_cast<const NumInternalSides &>(uo);
  _count += obj.count();
}

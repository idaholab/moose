/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


//  Base class for effective saturation as a function of pressure(s)
//
#include "RichardsSeff.h"

template<>
InputParameters validParams<RichardsSeff>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Fluid seff base class.  Override seff, dseff and d2seff in your class");
  return params;
}

RichardsSeff::RichardsSeff(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters)
{}

void
RichardsSeff::initialize()
{}

void
RichardsSeff::execute()
{}

void RichardsSeff::finalize()
{}


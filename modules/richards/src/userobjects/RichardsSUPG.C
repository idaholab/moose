/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  Base class for Richards SUPG
//
#include "RichardsSUPG.h"

template <>
InputParameters
validParams<RichardsSUPG>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Richards SUPG base class.  Override tauSUPG, etc");
  return params;
}

RichardsSUPG::RichardsSUPG(const InputParameters & parameters) : GeneralUserObject(parameters) {}

void
RichardsSUPG::initialize()
{
}

void
RichardsSUPG::execute()
{
}

void
RichardsSUPG::finalize()
{
}

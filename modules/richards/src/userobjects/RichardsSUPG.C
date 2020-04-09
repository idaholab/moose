//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  Base class for Richards SUPG
//
#include "RichardsSUPG.h"

InputParameters
RichardsSUPG::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
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

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"
#include "MooseTypes.h"
#include "ADReal.h"
#include "libmesh/fparser_ad.hh"

class ADFParser : public FunctionParserAD
{
public:
  ADFParser();
  ADFParser(const ADFParser & cpy);

  bool JITCompile();

  Real Eval(const Real *) { mooseError("Not implemented."); }
  ADReal Eval(const ADReal * Vars);

protected:
  const Real _epsilon;
};

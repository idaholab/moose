//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PARSEDMATERIAL_H
#define PARSEDMATERIAL_H

#include "ParsedMaterialHelper.h"
#include "FunctionMaterialBase.h"
#include "ParsedMaterialBase.h"

// Forward Declarations
class ParsedMaterial;

template <>
InputParameters validParams<ParsedMaterial>();

/**
 * FunctionMaterialBase child class to evaluate a parsed function. The function
 * can access non-linear and aux variables (unlike MooseParsedFunction).
 */
class ParsedMaterial : public ParsedMaterialHelper, public ParsedMaterialBase
{
public:
  ParsedMaterial(const InputParameters & parameters);
};

#endif // PARSEDMATERIAL_H

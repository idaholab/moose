//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParsedMaterialHelper.h"
#include "FunctionMaterialBase.h"
#include "ParsedMaterialBase.h"

/**
 * FunctionMaterialBase child class to evaluate a parsed function. The function
 * can access non-linear and aux variables (unlike MooseParsedFunction).
 */
template <bool is_ad>
class ParsedMaterialTempl : public ParsedMaterialHelper<is_ad>, public ParsedMaterialBase
{
public:
  static InputParameters validParams();

  ParsedMaterialTempl(const InputParameters & parameters);

  usingParsedMaterialHelperMembers(is_ad);
};

typedef ParsedMaterialTempl<false> ParsedMaterial;
typedef ParsedMaterialTempl<true> ADParsedMaterial;

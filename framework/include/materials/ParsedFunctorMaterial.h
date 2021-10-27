//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParsedFunctorMaterialHelper.h"
#include "FunctionMaterialBase.h"
#include "ParsedMaterialBase.h"

/**
 * FunctionMaterialBase child class to evaluate a parsed function. The function
 * can access non-linear and aux variables (unlike MooseParsedFunction).
 */
template <bool is_ad>
class ParsedFunctorMaterialTempl : public ParsedFunctorMaterialHelper<is_ad>, public ParsedMaterialBase
{
public:
  static InputParameters validParams();

  ParsedFunctorMaterialTempl(const InputParameters & parameters);

  usingParsedFunctorMaterialHelperMembers(is_ad);
  using FunctorMaterial::_mesh;
  using FunctorMaterial::blockIDs;
};

typedef ParsedFunctorMaterialTempl<false> ParsedFunctorMaterial;
typedef ParsedFunctorMaterialTempl<true> ADParsedFunctorMaterial;

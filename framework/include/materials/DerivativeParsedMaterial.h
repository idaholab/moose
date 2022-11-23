//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeParsedMaterialHelper.h"
#include "ParsedMaterialBase.h"

/**
 * Class to evaluate a parsed function (for example a free energy) and automatically
 * provide all derivatives.
 */
template <bool is_ad>
class DerivativeParsedMaterialTempl : public DerivativeParsedMaterialHelperTempl<is_ad>,
                                      public ParsedMaterialBase
{
public:
  static InputParameters validParams();

  DerivativeParsedMaterialTempl(const InputParameters & parameters);

  usingDerivativeParsedMaterialHelperMembers(is_ad);

protected:
  virtual void resetQpProperties() override {}
};

typedef DerivativeParsedMaterialTempl<false> DerivativeParsedMaterial;
typedef DerivativeParsedMaterialTempl<true> ADDerivativeParsedMaterial;

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

#include <unordered_map>

template <bool is_ad>
class FVPropValPerSubdomainMaterialTempl : public FunctorMaterial
{
public:
  FVPropValPerSubdomainMaterialTempl(const InputParameters & parameters);
  static InputParameters validParams();

private:
  FunctorMaterialProperty<GenericReal<is_ad>> & _prop;
};

typedef FVPropValPerSubdomainMaterialTempl<false> FVPropValPerSubdomainMaterial;
typedef FVPropValPerSubdomainMaterialTempl<true> FVADPropValPerSubdomainMaterial;

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "FunctorMaterial.h"
#include "FixedElemValueFunctor.h"

template <bool is_ad>
class FixedElemValueFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FixedElemValueFunctorMaterialTempl(const InputParameters & parameters);

protected:
  std::unique_ptr<FixedElemValueFunctor<GenericReal<is_ad>>> _ef;
  using FaceArg = Moose::FaceArg;
};

typedef FixedElemValueFunctorMaterialTempl<false> FixedElemValueFunctorMaterial;
typedef FixedElemValueFunctorMaterialTempl<true> ADFixedElemValueFunctorMaterial;

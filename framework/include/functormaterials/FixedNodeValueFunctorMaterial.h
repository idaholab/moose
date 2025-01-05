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
#include "FixedNodeValueFunctor.h"

template <bool is_ad>
class FixedNodeValueFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FixedNodeValueFunctorMaterialTempl(const InputParameters & parameters);

protected:
  std::unique_ptr<FixedNodeValueFunctor<GenericReal<is_ad>>> _nf;
  using FaceArg = Moose::FaceArg;
};

typedef FixedNodeValueFunctorMaterialTempl<false> FixedNodeValueFunctorMaterial;
typedef FixedNodeValueFunctorMaterialTempl<true> ADFixedNodeValueFunctorMaterial;

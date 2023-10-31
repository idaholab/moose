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
#include "BoundaryIntegrationFunctor.h"

template <bool is_ad>
class BoundaryIntegralFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  BoundaryIntegralFunctorMaterialTempl(const InputParameters & parameters);

protected:
  std::unique_ptr<BoundaryIntegralFunctor<GenericReal<is_ad>>> _bif;
  std::unique_ptr<BoundaryAverageFunctor<GenericReal<is_ad>>> _baf;
  using FaceArg = Moose::FaceArg;

  /// Factor multiplying the boundary integral / average
  const Moose::Functor<GenericReal<is_ad>> & _factor;
};

typedef BoundaryIntegralFunctorMaterialTempl<false> BoundaryIntegralFunctorMaterial;
typedef BoundaryIntegralFunctorMaterialTempl<true> ADBoundaryIntegralFunctorMaterial;

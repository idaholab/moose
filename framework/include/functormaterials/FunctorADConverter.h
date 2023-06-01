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

/**
 * This material converts regular functors to AD functors and AD functors to regular functors
 */
template <typename T>
class FunctorADConverterTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FunctorADConverterTempl(const InputParameters & parameters);
};

typedef FunctorADConverterTempl<Real> FunctorADConverter;
typedef FunctorADConverterTempl<RealVectorValue> VectorFunctorADConverter;

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"
#include "RankTwoTensor.h"

/**
 * This postprocessor computes an element integral of a
 * component of a material tensor as specified by the user-supplied indices.
 */
template <bool is_ad>
class MaterialTensorIntegralTempl : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  MaterialTensorIntegralTempl(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

private:
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _tensor;
  const unsigned int _i;
  const unsigned int _j;
};

typedef MaterialTensorIntegralTempl<false> MaterialTensorIntegral;
typedef MaterialTensorIntegralTempl<true> ADMaterialTensorIntegral;

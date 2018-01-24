//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALTENSORINTEGRAL_H
#define MATERIALTENSORINTEGRAL_H

#include "ElementIntegralPostprocessor.h"
#include "RankTwoTensor.h"

// Forward Declarations
class MaterialTensorIntegral;

template <>
InputParameters validParams<MaterialTensorIntegral>();

/**
 * This postprocessor computes an element integral of a
 * component of a material tensor as specified by the user-supplied indices.
 */
class MaterialTensorIntegral : public ElementIntegralPostprocessor
{
public:
  MaterialTensorIntegral(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

private:
  const MaterialProperty<RankTwoTensor> & _tensor;
  const unsigned int _i;
  const unsigned int _j;
};
#endif // MATERIALTENSORINTEGRAL_H

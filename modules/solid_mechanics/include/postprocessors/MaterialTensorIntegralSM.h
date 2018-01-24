//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALTENSORINTEGRALSM_H
#define MATERIALTENSORINTEGRALSM_H

#include "ElementIntegralPostprocessor.h"
#include "MaterialTensorCalculator.h"

// Forward Declarations
class MaterialTensorIntegralSM;
class SymmTensor;

template <>
InputParameters validParams<MaterialTensorIntegralSM>();

/**
 * This postprocessor computes an element integral of a
 * component of a material tensor
 */
class MaterialTensorIntegralSM : public ElementIntegralPostprocessor
{
public:
  MaterialTensorIntegralSM(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  MaterialTensorCalculator _material_tensor_calculator;
  const MaterialProperty<SymmTensor> & _tensor;
};

#endif // MATERIALTENSORINTEGRALSM_H

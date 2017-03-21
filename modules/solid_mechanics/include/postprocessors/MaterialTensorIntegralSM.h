/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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

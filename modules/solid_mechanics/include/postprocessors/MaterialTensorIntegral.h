/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef MATERIALTENSORINTEGRAL_H
#define MATERIALTENSORINTEGRAL_H

#include "ElementIntegralPostprocessor.h"
#include "MaterialTensorCalculator.h"

//Forward Declarations
class MaterialTensorIntegral;
class SymmTensor;

template<>
InputParameters validParams<MaterialTensorIntegral>();

/**
 * This postprocessor computes an element integral of a
 * component of a material tensor
 */
class MaterialTensorIntegral: public ElementIntegralPostprocessor
{
public:
  MaterialTensorIntegral(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  MaterialTensorCalculator _material_tensor_calculator;
  const MaterialProperty<SymmTensor> & _tensor;
};

#endif

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GBDEPENDENTTENSORBASE_H
#define GBDEPENDENTTENSORBASE_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensor.h"

class GBDependentTensorBase;

template <>
InputParameters validParams<GBDependentTensorBase>();
/**
 * Base class to define GB dependent properties
 */
class GBDependentTensorBase : public DerivativeMaterialInterface<Material>
{
public:
  GBDependentTensorBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() = 0;
  virtual void computeQpProperties() = 0;

  const VariableValue & _gb;
  Real _bulk_parameter;
  Real _gb_parameter;

  const MaterialProperty<RankTwoTensor> & _gb_normal_tensor;
  MaterialProperty<RealTensorValue> & _gb_dependent_tensor;
};

#endif

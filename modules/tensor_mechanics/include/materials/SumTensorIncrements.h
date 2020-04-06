//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * SumTensorIncrements update a tensor by summing tensor increments passed as property
 */
class SumTensorIncrements : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  SumTensorIncrements(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  std::vector<MaterialPropertyName> _property_names;
  unsigned int _num_property;

  MaterialProperty<RankTwoTensor> & _tensor;
  const MaterialProperty<RankTwoTensor> & _tensor_old;
  MaterialProperty<RankTwoTensor> & _tensor_increment;

  std::vector<const MaterialProperty<RankTwoTensor> *> _coupled_tensor_increments;
};

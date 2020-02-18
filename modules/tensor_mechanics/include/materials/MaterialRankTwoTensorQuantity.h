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

class MaterialRankTwoTensorQuantity;

/**
 * MaterialRankTwoTensorQuantity is designed to take the data in the RankTwoTensor material
 * property, for example stress or strain, and output the value for the
 * supplied indices.
 */

template <>
InputParameters validParams<MaterialRankTwoTensorQuantity>();

class MaterialRankTwoTensorQuantity : public Material
{
public:
  static InputParameters validParams();

  MaterialRankTwoTensorQuantity(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

private:
  const MaterialProperty<RankTwoTensor> & _tensor;
  const std::string _calculation_name;

  MaterialProperty<Real> & _calculation;

  const unsigned int _i;
  const unsigned int _j;
};

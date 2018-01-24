//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GENERICCONSTANTRANKTWOTENSOR_H
#define GENERICCONSTANTRANKTWOTENSOR_H

#include "Material.h"
#include "RankTwoTensor.h"

class GenericConstantRankTwoTensor;

template <>
InputParameters validParams<GenericConstantRankTwoTensor>();

/**
 * Declares a constant material property of type RankTwoTensor.
 */
class GenericConstantRankTwoTensor : public Material
{
public:
  GenericConstantRankTwoTensor(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  RankTwoTensor _tensor;
  MaterialProperty<RankTwoTensor> & _prop;
};

#endif // GENERICCONSTANTRANKTWOTENSOR_H

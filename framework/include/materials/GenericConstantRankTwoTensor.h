/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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

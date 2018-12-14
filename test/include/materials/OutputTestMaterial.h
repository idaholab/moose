//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef OUTPUTTESTMATERIAL_H
#define OUTPUTTESTMATERIAL_H

// MOOSE includes
#include "Material.h"

// Forward declarations
class OutputTestMaterial;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;
template <typename>
class RankFourTensorTempl;
typedef RankFourTensorTempl<Real> RankFourTensor;

template <>
InputParameters validParams<OutputTestMaterial>();

/**
 *
 */
class OutputTestMaterial : public Material
{
public:
  /**
   * Class constructor
   * @param prop_name
   */
  OutputTestMaterial(const InputParameters & parameters);

  // Used for testing if hidden compiler warning shows up
  virtual void computeProperties() { Material::computeProperties(); }

  /**
   * Class destructor
   */
  virtual ~OutputTestMaterial();

protected:
  virtual void computeQpProperties();

  MaterialProperty<Real> & _real_property;
  MaterialProperty<RealVectorValue> & _vector_property;
  MaterialProperty<RealTensorValue> & _tensor_property;
  MaterialProperty<RankTwoTensor> & _ranktwotensor_property;
  MaterialProperty<RankFourTensor> & _rankfourtensor_property;
  Real _factor;
  const VariableValue & _variable;
};

#endif // OUTPUTTESTMATERIAL_H

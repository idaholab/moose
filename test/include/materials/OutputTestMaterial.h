//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Material.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"

class OutputTestMaterial : public Material
{
public:
  /**
   * Class constructor
   * @param prop_name
   */
  static InputParameters validParams();

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
  MaterialProperty<std::vector<Real>> * _stdvector_property;
  Real _factor;
  const VariableValue & _variable;
};

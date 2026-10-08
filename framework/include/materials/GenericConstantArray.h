//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "VariableSizeMaterialPropertiesInterface.h"

class GenericConstantArray : public Material, public VariableSizeMaterialPropertiesInterface
{
public:
  static InputParameters validParams();

  GenericConstantArray(const InputParameters & parameters);

  virtual std::size_t getVectorPropertySize(const MaterialPropertyName & prop_name) const override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  std::string _prop_name;
  const RealEigenVector & _prop_value;

  MaterialProperty<RealEigenVector> & _property;
};

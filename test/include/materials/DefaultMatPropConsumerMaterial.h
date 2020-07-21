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
#include "DerivativeMaterialInterface.h"

template <bool is_ad>
class DefaultMatPropConsumerMaterialTempl : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  DefaultMatPropConsumerMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  std::string _prop_name;
  const GenericMaterialProperty<Real, is_ad> & _prop;
};

typedef DefaultMatPropConsumerMaterialTempl<false> DefaultMatPropConsumerMaterial;
typedef DefaultMatPropConsumerMaterialTempl<true> ADDefaultMatPropConsumerMaterial;

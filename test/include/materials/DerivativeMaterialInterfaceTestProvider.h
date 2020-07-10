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

/**
 * Test class that provides a few material properties through DerivativeMaterialInterface
 */
template <bool is_ad>
class DerivativeMaterialInterfaceTestProviderTempl : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  DerivativeMaterialInterfaceTestProviderTempl(const InputParameters & parameters);

  virtual void computeQpProperties();

protected:
  GenericMaterialProperty<Real, is_ad> &_prop1, &_prop2, &_prop3, &_prop4, &_prop5;
  MaterialProperty<dof_id_type> & _prop6;
  GenericMaterialProperty<Real, is_ad> & _prop7;
};

typedef DerivativeMaterialInterfaceTestProviderTempl<false> DerivativeMaterialInterfaceTestProvider;
typedef DerivativeMaterialInterfaceTestProviderTempl<true>
    ADDerivativeMaterialInterfaceTestProvider;

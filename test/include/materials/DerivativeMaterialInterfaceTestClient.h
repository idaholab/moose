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
class DerivativeMaterialInterfaceTestClientTempl : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  DerivativeMaterialInterfaceTestClientTempl(const InputParameters & parameters);

  virtual void initialSetup();
  virtual void computeQpProperties();

protected:
  MaterialPropertyName _prop_name;
  bool _by_name;
  const GenericMaterialProperty<Real, is_ad> &_prop0, &_prop1, &_prop2, &_prop3, &_prop4, &_prop5;
  const MaterialProperty<dof_id_type> & _prop6;
  const GenericMaterialProperty<Real, is_ad> &_prop7, &_prop8;
};

typedef DerivativeMaterialInterfaceTestClientTempl<false> DerivativeMaterialInterfaceTestClient;
typedef DerivativeMaterialInterfaceTestClientTempl<true> ADDerivativeMaterialInterfaceTestClient;

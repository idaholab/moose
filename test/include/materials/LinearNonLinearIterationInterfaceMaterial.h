//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMaterial.h"

/**
 * A special interface material used to check the ability of other MOOSE systems to get the correct
 * material property value ar each linear and non linear iteration. When using this material for
 * testing don't use Exodiff type tests.  For an example use see
 * InterfaceUserObjectTestGetMaterialProperty and realted tests.
 */
class LinearNonLinearIterationInterfaceMaterial : public InterfaceMaterial
{
public:
  static InputParameters validParams();

  LinearNonLinearIterationInterfaceMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  MaterialProperty<Real> & _mat_prop;
  const Real _prefactor;
};

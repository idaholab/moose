//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericConstantMaterial.h"

/**
 * A material that tracks the number of times computeQpProperties has been called.
 */
class IncrementMaterial : public GenericConstantMaterial
{
public:
  static InputParameters validParams();

  IncrementMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  unsigned int _inc;
  MaterialProperty<Real> & _mat_prop;
};

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

/**
 * A test object that uses optional material properties
 */
class OptionalTestMaterial : public Material
{
public:
  static InputParameters validParams();

  OptionalTestMaterial(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

private:
  const MaterialProperty<Real> * const & _prop;
  const ADMaterialProperty<Real> * const & _adprop;
  const bool _expect;
  const bool _adexpect;
  MaterialProperty<Real> & _mirror;
};

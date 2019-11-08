//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * This AuxKernel sets an elemental aux field to one on blocks where
 * a material property is defined and zero elsewhere. This class
 * throws an error if used on a Lagrange basis.
 */
class MaterialPropertyBlockAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MaterialPropertyBlockAux(const InputParameters & params);

protected:
  virtual void subdomainSetup() override;
  virtual Real computeValue() override;

  const MaterialPropertyName & _mat_prop_name;
  bool _has_prop;
};

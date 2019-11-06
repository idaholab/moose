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
#include "AuxKernel.h"

/**
 * Test class for testing functionality of getMaterialPropertyOld/Older for AuxKernels
 */
class OldMaterialAux : public AuxKernel
{
public:
  static InputParameters validParams();

  OldMaterialAux(const InputParameters & parameters);
  virtual ~OldMaterialAux() {}

protected:
  virtual Real computeValue();

private:
  const MaterialProperty<Real> & _old;
  const MaterialProperty<Real> & _older;
};

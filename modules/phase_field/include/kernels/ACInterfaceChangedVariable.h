//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/// Considers cleavage plane anisotropy in the crack propagation

#pragma once

#include "ACInterface.h"

class ACInterfaceChangedVariable : public ACInterface
{
public:
  static InputParameters validParams();

  ACInterfaceChangedVariable(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// Order parameter derivative
  const MaterialProperty<Real> & _dopdu;

  /// 2nd order parameter derivative
  const MaterialProperty<Real> & _d2opdu2;
};

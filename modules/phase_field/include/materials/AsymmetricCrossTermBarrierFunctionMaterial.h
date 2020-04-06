//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrossTermBarrierFunctionBase.h"

// Forward Declarations

/**
 * AsymmetricCrossTermBarrierFunctionMaterial adds a free energy contribution on the
 * interfaces between arbitrary pairs of phases in an asymmetric way, allowing to tune the
 * magnitude of the free energy density cotribution on both sides of the interface independently.
 */
class AsymmetricCrossTermBarrierFunctionMaterial : public CrossTermBarrierFunctionBase
{
public:
  static InputParameters validParams();

  AsymmetricCrossTermBarrierFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  ///@{ Switching functions and their drivatives
  std::vector<const MaterialProperty<Real> *> _h;
  std::vector<const MaterialProperty<Real> *> _dh;
  std::vector<const MaterialProperty<Real> *> _d2h;
  ///@}
};

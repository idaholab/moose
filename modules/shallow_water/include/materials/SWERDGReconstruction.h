//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "SlopeLimitingBase.h"

/**
 * Face-extrapolated values for SWE variables [h, hu, hv].
 *
 * Stub: currently passes through cell-average values at faces.
 * Can be extended to use RDG slope reconstruction and limiting.
 */
class SWERDGReconstruction : public Material
{
public:
  static InputParameters validParams();

  SWERDGReconstruction(const InputParameters & parameters);
  virtual ~SWERDGReconstruction();

protected:
  virtual void computeQpProperties() override;

  // cell-average values
  const VariableValue & _h;
  const VariableValue & _hu;
  const VariableValue & _hv;

  // face-extrapolated values
  MaterialProperty<Real> & _hf;
  MaterialProperty<Real> & _huf;
  MaterialProperty<Real> & _hvf;

  // slope limiting user object (multi-D)
  const SlopeLimitingBase * _lslope;
};

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSFVHLLCStagnationInletBC.h"

/**
 * HLLC stagnation inlet boundary conditions for the conservation of energy equation
 */
class CNSFVHLLCFluidEnergyStagnationInletBC : public CNSFVHLLCStagnationInletBC
{
public:
  CNSFVHLLCFluidEnergyStagnationInletBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal fluxElem() override;
  virtual ADReal fluxBoundary() override;
  virtual ADReal hllcElem() override;
  virtual ADReal hllcBoundary() override;
  virtual ADReal conservedVariableElem() override;
  virtual ADReal conservedVariableBoundary() override;

  /// specific total enthalpy material property on elem side
  const ADMaterialProperty<Real> & _ht_elem;
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSFVHLLCImplicitBC.h"

/**
 * HLLC implicit boundary conditions for the energy conservation equation
 */
class CNSFVHLLCFluidEnergyImplicitBC : public CNSFVHLLCImplicitBC
{
public:
  CNSFVHLLCFluidEnergyImplicitBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal fluxElem() override;
  virtual ADReal hllcElem() override;
  virtual ADReal conservedVariableElem() override;

  /// specific total enthalpy material property on elem side
  const ADMaterialProperty<Real> & _ht_elem;
};

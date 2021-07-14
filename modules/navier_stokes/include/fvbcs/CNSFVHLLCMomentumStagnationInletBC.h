//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSFVHLLCStagnationInletBC.h"

/**
 * HLLC stagnation inlet boundary conditions for the momentum conservation equation
 */
class CNSFVHLLCMomentumStagnationInletBC : public CNSFVHLLCStagnationInletBC
{
public:
  CNSFVHLLCMomentumStagnationInletBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal fluxElem() override;
  virtual ADReal fluxBoundary() override;
  virtual ADReal hllcElem() override;
  virtual ADReal hllcBoundary() override;
  virtual ADReal conservedVariableElem() override;
  virtual ADReal conservedVariableBoundary() override;

  /// index x|y|z
  unsigned int _index;
};

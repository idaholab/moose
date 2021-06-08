//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSFVHLLCBCBase.h"
#include "CNSFVHLLCBase.h"

class SinglePhaseFluidProperties;

/**
 * Base clase for HLLC boundary condition for Euler equation
 */
class CNSFVHLLCBC : public CNSFVHLLCBCBase
{
public:
  CNSFVHLLCBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  ///@{ flux functions on elem & from boundary
  virtual ADReal fluxElem() = 0;
  virtual ADReal fluxBoundary() = 0;
  ///@}

  ///@{ HLLC modifications to flux for elem & boundary, see Toro
  virtual ADReal hllcElem() = 0;
  virtual ADReal hllcBoundary() = 0;
  ///@}

  ///@{ conserved variable of this equation from elem and boundary
  virtual ADReal conservedVariableElem() = 0;
  virtual ADReal conservedVariableBoundary() = 0;
  ///@}
};

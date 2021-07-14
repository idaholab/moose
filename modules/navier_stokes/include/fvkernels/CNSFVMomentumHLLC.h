//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSFVHLLC.h"

/**
 * Implements the advective flux and the pressure terms in the conservation of momentum equation
 * using a HLLC discretization
 */
class CNSFVMomentumHLLC : public CNSFVHLLC
{
public:
  static InputParameters validParams();
  CNSFVMomentumHLLC(const InputParameters & params);

protected:
  ///@{ flux functions on elem & neighbor, i.e. standard left/right values of F
  virtual ADReal fluxElem() override;
  virtual ADReal fluxNeighbor() override;
  ///@}

  ///@{ HLLC modifications to flux for elem & neighbor, see Toro
  virtual ADReal hllcElem() override;
  virtual ADReal hllcNeighbor() override;
  ///@}

  ///@{ conserved variable of this equation This is not just _u_elem/_u_neighbor
  /// to allow using different sets of variables in the future
  virtual ADReal conservedVariableElem() override;
  virtual ADReal conservedVariableNeighbor() override;
  ///@}

  /// index x|y|z
  unsigned int _index;
};

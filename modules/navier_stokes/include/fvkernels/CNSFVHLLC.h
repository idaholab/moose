//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSFVHLLCBase.h"

/**
 * Base class for HLLC inter-cell flux kernels. This class defines the interfaces necessary to
 * complete the definition of the HLLC fluxes in derived mass, momentum, and fluid energy classes.
 * It also selects the HLLC flux computation branch based on the wave speeds
 */
class CNSFVHLLC : public CNSFVHLLCBase
{
public:
  static InputParameters validParams();
  CNSFVHLLC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  ///@{ flux functions on elem & neighbor, i.e. standard left/right values of F
  virtual ADReal fluxElem() = 0;
  virtual ADReal fluxNeighbor() = 0;
  ///@}

  ///@{ HLLC modifications to flux for elem & neighbor, see Toro
  virtual ADReal hllcElem() = 0;
  virtual ADReal hllcNeighbor() = 0;
  ///@}

  ///@{ conserved variable of this equation This is not just _u_elem/_u_neighbor
  /// to allow using different sets of variables in the future
  virtual ADReal conservedVariableElem() = 0;
  virtual ADReal conservedVariableNeighbor() = 0;
  ///@}
};

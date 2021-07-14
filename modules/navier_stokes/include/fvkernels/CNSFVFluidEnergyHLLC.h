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
 * Implements the advective flux in the conservation of fluid energy equation using a HLLC
 * discretization
 */
class CNSFVFluidEnergyHLLC : public CNSFVHLLC
{
public:
  static InputParameters validParams();
  CNSFVFluidEnergyHLLC(const InputParameters & params);

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

  ///@{ enthalpies left == elem, right == neighbor
  const ADMaterialProperty<Real> & _ht_elem;
  const ADMaterialProperty<Real> & _ht_neighbor;
  ///@}
};

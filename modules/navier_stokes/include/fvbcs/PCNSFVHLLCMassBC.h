//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC.h"
#include "PCNSFVHLLCSpecifiedPressureBC.h"

/**
 * Template class for implementing the advective flux in the porous conservation of mass equation at
 * boundaries when using a HLLC discretization
 */
template <typename T>
class PCNSFVHLLCMassBC : public T
{
public:
  static InputParameters validParams();
  PCNSFVHLLCMassBC(const InputParameters & params);

protected:
  ///@{ flux functions on elem & boundary, i.e. standard left/right values of F
  virtual ADReal fluxElem() override;
  virtual ADReal fluxBoundary() override;
  ///@}

  ///@{ HLLC modifications to flux for elem & boundary, see Toro
  virtual ADReal hllcElem() override;
  virtual ADReal hllcBoundary() override;
  ///@}

  ///@{ conserved variable of this equation This is not just _u_elem/_u_boundary
  /// to allow using different sets of variables in the future
  virtual ADReal conservedVariableElem() override;
  virtual ADReal conservedVariableBoundary() override;
  ///@}
};

typedef PCNSFVHLLCMassBC<PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC>
    PCNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC;
typedef PCNSFVHLLCMassBC<PCNSFVHLLCSpecifiedPressureBC> PCNSFVHLLCSpecifiedPressureMassBC;

template <>
InputParameters PCNSFVHLLCMassBC<PCNSFVHLLCSpecifiedPressureBC>::validParams();

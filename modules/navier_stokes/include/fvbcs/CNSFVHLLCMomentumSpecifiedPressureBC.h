//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSFVHLLCMomentumImplicitBC.h"

class Function;

/**
 * HLLC pressure boundary conditions for the momentum conservation equation
 */
class CNSFVHLLCMomentumSpecifiedPressureBC : public CNSFVHLLCMomentumImplicitBC
{
public:
  CNSFVHLLCMomentumSpecifiedPressureBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ///@{ both flux functions must be overriden to use given pressure
  virtual ADReal fluxElem() override;
  virtual ADReal fluxBoundary() override;
  ///@}

  /// function providing the pressure
  const Function & _pressure_function;
};

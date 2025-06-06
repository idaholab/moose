//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class ADShaftConnectedCompressor1PhaseUserObject;

/**
 * Computes various quantities for a ShaftConnectedCompressor1Phase.
 */
class ShaftConnectedCompressor1PhaseAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ShaftConnectedCompressor1PhaseAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Quantity type
  enum class Quantity
  {
    DELTA_P,
    ISENTROPIC_TORQUE,
    DISSIPATION_TORQUE,
    FRICTION_TORQUE,
    MOMENT_OF_INERTIA
  };
  /// Which quantity to compute
  const Quantity _quantity;

  /// 1-phase shaft-connected compressor user object
  const ADShaftConnectedCompressor1PhaseUserObject & _compressor_uo;
};

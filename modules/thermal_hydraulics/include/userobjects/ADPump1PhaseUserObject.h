//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVolumeJunction1PhaseUserObject.h"

/**
 * Computes and caches flux and residual vectors for a 1-phase pump
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the pump, and
 * \li fluxes between the flow channels and the pump.
 */
class ADPump1PhaseUserObject : public ADVolumeJunction1PhaseUserObject
{
public:
  ADPump1PhaseUserObject(const InputParameters & params);

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  /// Pump head, [m]
  const Real & _head;
  /// Gravity constant, i.e., 9.8 [m/s^2]
  const Real & _g;

public:
  static InputParameters validParams();
};

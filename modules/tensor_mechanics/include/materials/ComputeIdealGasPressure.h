//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeGasPressureBase.h"

/**
 * Sets up an isotropic extra Stress with a trace that equals the pressure of
 * an ideal gas with the given density.
 * \f$ PV=Nk_BT \f$
 */
class ComputeIdealGasPressure : public ComputeGasPressureBase
{
public:
  static InputParameters validParams();

  ComputeIdealGasPressure(const InputParameters & parameters);

protected:
  virtual void computeQpExtraStress();
};

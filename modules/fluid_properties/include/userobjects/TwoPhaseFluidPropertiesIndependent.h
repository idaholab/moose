//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TWOPHASEFLUIDPROPERTIESINDEPENDENT_H
#define TWOPHASEFLUIDPROPERTIESINDEPENDENT_H

#include "TwoPhaseFluidProperties.h"

class TwoPhaseFluidPropertiesIndependent;

template <>
InputParameters validParams<TwoPhaseFluidPropertiesIndependent>();

/**
 * 2-phase fluid properties for 2 independent single-phase fluid properties.
 *
 * This class throws errors if any 2-phase interfaces are called.
 */
class TwoPhaseFluidPropertiesIndependent : public TwoPhaseFluidProperties
{
public:
  TwoPhaseFluidPropertiesIndependent(const InputParameters & parameters);

  virtual Real p_critical() const override;
  virtual Real T_sat(Real p) const override;
  virtual Real p_sat(Real T) const override;
  virtual Real dT_sat_dp(Real p) const override;

  virtual bool supportsPhaseChange() const override { return false; }

  /**
   * Calls \c mooseError with a message saying that this class cannot call
   * 2-phase fluid properties.
   */
  [[noreturn]] void throwNotImplementedError() const;
};

#endif /* TWOPHASEFLUIDPROPERTIESINDEPENDENT_H */

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TwoPhaseFluidProperties.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * 2-phase fluid properties for 2 independent single-phase fluid properties.
 *
 * By default, this class throws errors if any 2-phase interfaces are called.
 */
class TwoPhaseFluidPropertiesIndependent : public TwoPhaseFluidProperties
{
public:
  static InputParameters validParams();

  TwoPhaseFluidPropertiesIndependent(const InputParameters & parameters);

  virtual Real p_critical() const override;
  virtual Real T_triple() const override;
  virtual Real T_sat(Real p) const override;
  virtual DualReal T_sat(const DualReal & p) const override;
  virtual Real p_sat(Real T) const override;
  virtual DualReal p_sat(const DualReal & T) const override;
  virtual Real dT_sat_dp(Real p) const override;
  virtual Real L_fusion() const override;
  virtual Real sigma_from_T(Real T) const override;
  virtual DualReal sigma_from_T(const DualReal & T) const override;
  virtual Real dsigma_dT_from_T(Real T) const override;

  virtual bool supportsPhaseChange() const override { return false; }

protected:
  /**
   * Returns a dummy zero value or throws a \c mooseError.
   */
  Real getTwoPhaseInterfaceDummyValue() const;

  /// If true, throw an error when a 2-phase interface is called. Else, return a zero value.
  const bool _error_on_unimplemented;
};

#pragma GCC diagnostic pop

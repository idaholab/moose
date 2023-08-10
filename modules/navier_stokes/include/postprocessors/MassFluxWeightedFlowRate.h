//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "VolumetricFlowRate.h"
#include "MathFVUtils.h"

class SinglePhaseFluidProperties;

/**
 * This postprocessor computes the mass-flux weighted average of a flow
 * quantity over a boundary, internal or external to the flow domain.
 */
class MassFluxWeightedFlowRate : public VolumetricFlowRate
{
public:
  static InputParameters validParams();

  MassFluxWeightedFlowRate(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;
  using Postprocessor::getValue;
  virtual Real getValue() const override;

protected:
  virtual Real computeFaceInfoIntegral(const FaceInfo * fi) override;

  /// density provided as functor
  const Moose::Functor<ADReal> & _density;

  /// the mass flow rate over the face computed by this object
  Real _mdot;
};

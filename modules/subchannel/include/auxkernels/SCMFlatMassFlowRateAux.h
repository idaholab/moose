//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "SubChannelMesh.h"

/**
 * Computes a uniform mass flow rate at the inlet provided a total mass flow rate
 */
class SCMFlatMassFlowRateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  SCMFlatMassFlowRateAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// Specified mass flow
  const Real & _mass_flow;
  /// Geometry information
  const SubChannelMesh & _subchannel_mesh;
};

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
 * Computes mass float rate from specified uniform mass flux and cross-sectional area and applies
 * inlet blockage conditions
 */
class SCMBlockedMassFlowRateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  SCMBlockedMassFlowRateAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  SubChannelMesh & _subchannel_mesh;
  /// Specified mass flux
  const Real & _unblocked_mass_flux;
  /// Specified mass flow rate
  const Real & _blocked_mass_flux;
  /// Cross-sectional area
  const VariableValue & _area;
  /// index of subchannels affected by blockage
  std::vector<unsigned int> _index_blockage;
};

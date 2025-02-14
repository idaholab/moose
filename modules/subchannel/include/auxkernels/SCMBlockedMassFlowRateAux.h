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
#include "SubChannelMesh.h"

/**
 * Computes mass flow rate from specified uniform mass flux and cross-sectional area and applies
 * inlet blockage conditions
 */
class SCMBlockedMassFlowRateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  SCMBlockedMassFlowRateAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  const SubChannelMesh & _subchannel_mesh;
  /// Specified mass flux of unblocked channels
  const PostprocessorValue & _unblocked_mass_flux;
  /// Specified mass flux of blocked channels
  const PostprocessorValue & _blocked_mass_flux;
  /// Cross-sectional area
  const VariableValue & _area;
  /// index of subchannels affected by blockage
  const std::vector<unsigned int> _index_blockage;
};

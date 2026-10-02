//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

/**
 * Narrow, framework-agnostic contract that a "heat transfer connection" (something coupling a
 * flow channel to a wall temperature or heat flux) must satisfy to be usable by a ClosuresBase-
 * derived object's addMooseObjectsHeatTransfer()/checkHeatTransfer(). Mirrors
 * FlowChannelClosuresInterface's role for the flow-channel side - see its documentation.
 *
 * Classic THM's HeatTransferBase/HeatTransfer1PhaseBase implement this (near-zero cost, see
 * HeatTransferBase.h/.C). No ActionComponent implements it yet: there is no native heat-transfer-
 * to-flow-channel coupling component today (that is future work, analogous to
 * HeatTransferFromHeatStructure1Phase), so this interface exists now to keep ClosuresBase's shape
 * right, but has exactly one implementor until that component is built.
 */
class HeatTransferClosuresInterface
{
public:
  virtual ~HeatTransferClosuresInterface() = default;

  /// Name of the heat transfer connection, for naming the MOOSE objects closures create
  virtual const std::string & getClosuresName() const = 0;

  /// Whether a wall heat transfer coefficient function ('Hw') was provided
  virtual bool hasClosuresWallHeatTransferCoefficientFunction() const = 0;
  /// The wall heat transfer coefficient function ('Hw')
  virtual const FunctionName & getClosuresWallHeatTransferCoefficientFunction() const = 0;
  /// Name of the 1-phase wall heat transfer coefficient material property this connection uses
  virtual const MaterialPropertyName & getClosuresWallHeatTransferCoefficient1PhaseName() const = 0;
};

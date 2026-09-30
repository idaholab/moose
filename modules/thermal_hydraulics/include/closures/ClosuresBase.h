//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "LoggingInterface.h"
#include "NamingInterface.h"

class FlowChannelClosuresInterface;
class HeatTransferClosuresInterface;
class FEProblemBase;
class Factory;

/**
 * Base class for closures implementations
 *
 * The responsibilities of the closures objects depend on the flow model that
 * uses them. Examples of responsibilities will be to provide material properties
 * for friction factors and heat transfer coefficients.
 *
 * This class depends only on FlowChannelClosuresInterface/HeatTransferClosuresInterface and a
 * plain FEProblemBase, not on any classic THM Component or on THMProblem specifically - so the
 * same closures classes (this one and its subclasses) serve both classic Components (via
 * FlowChannelBase/HeatTransferBase, which implement those interfaces) and native
 * ActionComponents/Physics (e.g. FlowChannel1PhasePhysics, which implements
 * FlowChannelClosuresInterface directly) with no duplicated logic.
 */
class ClosuresBase : public MooseObject, public LoggingInterface, public NamingInterface
{
public:
  ClosuresBase(const InputParameters & params);

  /**
   * Checks for errors associated with a flow channel
   *
   * @param[in] flow_channel   Flow channel
   */
  virtual void checkFlowChannel(const FlowChannelClosuresInterface & /*flow_channel*/) const {}

  /**
   * Checks for errors associated with a heat transfer connection
   *
   * @param[in] heat_transfer   Heat transfer connection
   * @param[in] flow_channel   Flow channel
   */
  virtual void checkHeatTransfer(const HeatTransferClosuresInterface & /*heat_transfer*/,
                                 const FlowChannelClosuresInterface & /*flow_channel*/) const
  {
  }

  /**
   * Adds MOOSE objects associated with a flow channel
   *
   * @param[in] flow_channel   Flow channel
   */
  virtual void addMooseObjectsFlowChannel(const FlowChannelClosuresInterface & flow_channel) = 0;

  /**
   * Adds MOOSE objects associated with a heat transfer connection
   *
   * @param[in] heat_transfer   Heat transfer connection
   * @param[in] flow_channel   Flow channel
   */
  virtual void addMooseObjectsHeatTransfer(const HeatTransferClosuresInterface & heat_transfer,
                                           const FlowChannelClosuresInterface & flow_channel) = 0;

protected:
  /**
   * Adds an arbitrary zero-value material
   *
   * @param[in] flow_channel   Flow channel
   * @param[in] property_name   Name of the material property to create
   */
  void addZeroMaterial(const FlowChannelClosuresInterface & flow_channel,
                       const std::string & property_name) const;

  /**
   * Adds a weighted average material
   *
   * @param[in] flow_channel   Flow channel
   * @param[in] values   Values to average
   * @param[in] weights   Weights for each value
   * @param[in] property_name   Name of material property to create
   */
  void addWeightedAverageMaterial(const FlowChannelClosuresInterface & flow_channel,
                                  const std::vector<MaterialPropertyName> & values,
                                  const std::vector<VariableName> & weights,
                                  const MaterialPropertyName & property_name) const;

  /**
   * Adds a material for wall temperature from an aux variable
   *
   * @param[in] flow_channel   Flow channel
   * @param[in] i   index of the heat transfer
   */
  void addWallTemperatureFromAuxMaterial(const FlowChannelClosuresInterface & flow_channel,
                                         unsigned int i = 0) const;

  /// The problem to which closures-created MOOSE objects (materials, ...) are added
  FEProblemBase & _problem;

  /// Factory associated with the MooseApp
  Factory & _factory;

public:
  static InputParameters validParams();
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

class FlowChannelBase;
class HeatTransferBase;
class THMProblem;
class Factory;

/**
 * Base class for closures implementations
 *
 * The responsibilities of the closures objects depend on the flow model that
 * uses them. Examples of responsibilities will be to provide material properties
 * for friction factors and heat transfer coefficients.
 */
class ClosuresBase : public MooseObject, public LoggingInterface, public NamingInterface
{
public:
  ClosuresBase(const InputParameters & params);

  /**
   * Checks for errors associated with a flow channel component
   *
   * @param[in] flow_channel   Flow channel component
   */
  virtual void checkFlowChannel(const FlowChannelBase & /*flow_channel*/) const {}

  /**
   * Checks for errors associated with a heat transfer component
   *
   * @param[in] heat_transfer   Heat transfer component
   * @param[in] flow_channel   Flow channel component
   */
  virtual void checkHeatTransfer(const HeatTransferBase & /*heat_transfer*/,
                                 const FlowChannelBase & /*flow_channel*/) const
  {
  }

  /**
   * Adds MOOSE objects associated with a flow channel component
   *
   * @param[in] flow_channel   Flow channel component
   */
  virtual void addMooseObjectsFlowChannel(const FlowChannelBase & flow_channel) = 0;

  /**
   * Adds MOOSE objects associated with a heat transfer component
   *
   * @param[in] heat_transfer   Heat transfer component
   * @param[in] flow_channel   Flow channel component
   */
  virtual void addMooseObjectsHeatTransfer(const HeatTransferBase & heat_transfer,
                                           const FlowChannelBase & flow_channel) = 0;

protected:
  /**
   * Adds an arbitrary zero-value material
   *
   * @param[in] flow_channel   Flow channel component
   * @param[in] property_name   Name of the material property to create
   */
  void addZeroMaterial(const FlowChannelBase & flow_channel,
                       const std::string & property_name) const;

  /**
   * Adds a weighted average material
   *
   * @param[in] flow_channel   Flow channel component
   * @param[in] values   Values to average
   * @param[in] weights   Weights for each value
   * @param[in] property_name   Name of material property to create
   */
  void addWeightedAverageMaterial(const FlowChannelBase & flow_channel,
                                  const std::vector<MaterialPropertyName> & values,
                                  const std::vector<VariableName> & weights,
                                  const MaterialPropertyName & property_name) const;

  /**
   * Adds a material for wall temperature from an aux variable
   *
   * @param[in] flow_channel   Flow channel component
   * @param[in] i   index of the heat transfer
   */
  void addWallTemperatureFromAuxMaterial(const FlowChannelBase & flow_channel,
                                         unsigned int i = 0) const;

  /// Simulation
  THMProblem & _sim;

  /// Factory associated with the MooseApp
  Factory & _factory;

public:
  static InputParameters validParams();
};

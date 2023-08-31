//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Closures1PhaseBase.h"

/**
 * THM 1-phase closures
 */
class Closures1PhaseTHM : public Closures1PhaseBase
{
public:
  Closures1PhaseTHM(const InputParameters & params);

  /// Fluid heat transfer coefficient closure type
  enum class WallHTCClosureType
  {
    DITTUS_BOELTER,
    KAZIMI_CARELLI,
    LYON,
    MIKITYUK,
    SCHAD,
    WEISMAN,
    WOLF_MCCARTHY,
    GNIELINSKI
  };

  /// Fluid friction factor closure type
  enum class WallFFClosureType
  {
    CHENG_TODREAS,
    CHURCHILL
  };

  virtual void checkFlowChannel(const FlowChannelBase & flow_channel) const override;
  virtual void checkHeatTransfer(const HeatTransferBase & heat_transfer,
                                 const FlowChannelBase & flow_channel) const override;
  virtual void addMooseObjectsFlowChannel(const FlowChannelBase & flow_channel) override;
  virtual void addMooseObjectsHeatTransfer(const HeatTransferBase & /*heat_transfer*/,
                                           const FlowChannelBase & /*flow_channel*/) override{};

protected:
  /**
   * Adds material that computes wall friction factor
   *
   * @param[in] flow_channel   Flow channel component
   */
  void addWallFFMaterial(const FlowChannel1Phase & flow_channel) const;

  /**
   * Adds wall heat transfer coefficient material
   *
   * @param[in] flow_channel   Flow channel component
   * @param[in] i   Heat transfer index
   */
  void addWallHTCMaterial(const FlowChannel1Phase & flow_channel, unsigned int i) const;

  /**
   * Adds computation of wall temperature when heat flux is specified
   *
   * @param[in] flow_channel   Flow channel component
   * @param[in] i   Heat transfer index
   */
  void addTemperatureWallFromHeatFluxMaterial(const FlowChannel1Phase & flow_channel,
                                              unsigned int i) const;
  /// Wall heat transfer coefficient closure
  const WallHTCClosureType _wall_htc_closure;

  /// Wall friction factor closure
  const WallFFClosureType _wall_ff_closure;

public:
  static InputParameters validParams();
};

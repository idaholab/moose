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
 * Narrow, framework-agnostic contract that a "flow channel" must satisfy to be usable by a
 * ClosuresBase-derived object (see Closures1PhaseBase/Closures1PhaseSimple/Closures1PhaseTHM/
 * FunctorClosures).
 *
 * This exists so that closures objects (and any custom ones a user writes) depend only on this
 * interface, never on whether the flow channel is a classic THM Component (FlowChannelBase,
 * implementing this interface in FlowChannelBase.h/.C) or a native ActionComponent's Physics
 * (FlowChannel1PhasePhysics, implementing this interface directly). Neither implementation is
 * more "canonical" than the other; ClosuresBase itself no longer knows either type exists.
 */
class FlowChannelClosuresInterface
{
public:
  virtual ~FlowChannelClosuresInterface() = default;

  /// Type of convective heat transfer geometry (mirrors FlowChannelBase::EConvHeatTransGeom;
  /// defined here, not reused from there, to avoid a circular include between this interface and
  /// the concrete flow channel classes that implement it)
  enum class HeatTransferGeometry
  {
    PIPE,
    ROD_BUNDLE,
    HEX_ROD_BUNDLE
  };

  /// Pipe location within a bundle (mirrors FlowChannelBase::EPipeLocation; see above)
  enum class PipeLocation
  {
    INTERIOR,
    EDGE,
    CORNER
  };

  /// Blocks the flow channel occupies
  virtual const std::vector<SubdomainName> & getClosuresBlocks() const = 0;
  /// Name of the flow channel, for naming the MOOSE objects closures create
  virtual const std::string & getClosuresName() const = 0;

  /// Whether one or more heat transfer connections specify a wall temperature (as opposed to a
  /// heat flux)
  virtual bool getClosuresTemperatureMode() const = 0;
  /// Number of heat transfer connections to this flow channel
  virtual unsigned int getClosuresNumberOfHeatTransferConnections() const = 0;
  /// Names of the heated perimeter variables for each heat transfer connection
  virtual std::vector<VariableName> getClosuresHeatedPerimeterNames() const = 0;
  /// Names of the wall temperature variables for each heat transfer connection
  virtual std::vector<VariableName> getClosuresWallTemperatureNames() const = 0;
  /// Names of the 1-phase wall heat transfer coefficient material properties for each heat
  /// transfer connection. Defaults to empty - only a 1-phase flow channel with heat transfer
  /// connections needs to override this.
  virtual std::vector<MaterialPropertyName> getClosuresWallHTCNames() const { return {}; }

  /// Convective heat transfer geometry (pipe / rod bundle / hex rod bundle)
  virtual HeatTransferGeometry getClosuresHeatTransferGeometry() const = 0;
  /// Pipe location within a bundle (interior / edge / corner)
  virtual PipeLocation getClosuresPipeLocation() const = 0;

  /// Whether a wall friction factor function ('f') was provided
  virtual bool hasClosuresWallFrictionFactorFunction() const = 0;
  /// The wall friction factor function ('f')
  virtual const FunctionName & getClosuresWallFrictionFactorFunction() const = 0;
  /// Surface roughness
  virtual Real getClosuresRoughness() const = 0;
  /// Pitch-to-diameter ratio for parallel bundle heat transfer
  virtual Real getClosuresPoD() const = 0;

  /// Connects a controllable parameter of the flow channel to a controllable parameter of a
  /// MOOSE object a closures object created (e.g. the roughness of a wall friction material) -
  /// see Component::connectObject(), which this mirrors
  virtual void connectClosuresObject(const InputParameters & obj_params,
                                     const std::string & obj_name,
                                     const std::string & param) const = 0;
};

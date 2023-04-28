//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatTransferFromTemperature1Phase.h"
#include "MeshAlignment1D3D.h"
#include "MooseEnum.h"

/**
 * Connects a 1-phase flow channel and a 3D heat structure
 */
class HeatTransferFromHeatStructure3D1Phase : public HeatTransferFromTemperature1Phase
{
public:
  HeatTransferFromHeatStructure3D1Phase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// enum for axis alignment
  enum EAxisAlignment
  {
    INVALID = -1,
    X,
    Y,
    Z
  };

  virtual const FEType & getFEType() override;

  virtual void setupMesh() override;
  virtual void init() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  virtual EAxisAlignment getFlowChannelAxisAlignment(const std::string & flow_channel_name) const;

  /// name of the connected flow channel
  const std::vector<std::string> & _flow_channel_names;
  /// Boundary names for which the boundary component applies
  const BoundaryName & _boundary;
  /// Closures associated with each flow channel
  std::vector<std::shared_ptr<ClosuresBase>> _flow_channel_closures;
  /// Heat structure name
  const std::string & _hs_name;
  /// Mesh alignment object
  MeshAlignment1D3D _mesh_alignment;
  /// Number of layers in the flow channel direction
  unsigned int _num_layers;
  /// Direction for layered average user objects
  MooseEnum _layered_average_uo_direction;

public:
  static InputParameters validParams();
};

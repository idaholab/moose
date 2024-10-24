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
#include "HSBoundaryInterface.h"
#include "MeshAlignment.h"

/**
 * Connects a 1-phase flow channel and a heat structure
 */
class HeatTransferFromHeatStructure1Phase : public HeatTransferFromTemperature1Phase,
                                            public HSBoundaryInterface
{
public:
  HeatTransferFromHeatStructure1Phase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual const libMesh::FEType & getFEType() override;

  virtual void setupMesh() override;
  virtual void check() const override;

  /**
   * Gets the heat structure boundary name for this connection
   *
   * @return The boundary name of the heat structure side
   */
  const BoundaryName & getHeatStructureSideName() const;
  /**
   * Gets the flow channel nodeset name for this connection
   *
   * @return The nodeset name for the channel side
   */
  const BoundaryName & getChannelSideName() const;

  /// Mesh alignment
  MeshAlignment _mesh_alignment;

public:
  static InputParameters validParams();
};

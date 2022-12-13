//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometricalComponent.h"
#include "DiscreteLineSegmentInterface.h"

/**
 * Base class for components that generate their own mesh
 */
class GeneratedMeshComponent : public GeometricalComponent, public DiscreteLineSegmentInterface
{
public:
  GeneratedMeshComponent(const InputParameters & parameters);

protected:
  virtual void setupMesh() override;
  virtual void check() const override;

  virtual void buildMesh() = 0;

  /**
   * Check if second order mesh is being used by this geometrical component
   *
   * @return true if second order mesh is being used, otherwise false
   */
  virtual bool usingSecondOrderMesh() const = 0;

  /// Axial region names
  const std::vector<std::string> & _axial_region_names;

  /// Node locations along the main axis
  std::vector<Real> _node_locations;

private:
  void generateNodeLocations();
  unsigned int computeNumberOfNodes(unsigned int n_elems);
  std::vector<Real> getUniformNodeLocations(Real length, unsigned int n_nodes);
  void placeLocalNodeLocations(Real start_length,
                               unsigned int start_node,
                               std::vector<Real> & local_node_locations);

public:
  static InputParameters validParams();
};

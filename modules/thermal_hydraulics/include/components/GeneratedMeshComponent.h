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
  static InputParameters validParams();

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
  /**
   * Generates axial node locations and stores in \c _node_locations
   */
  void generateNodeLocations();

  /**
   * Computes the number of axial nodes from the number of elements
   *
   * @param[in] n_elems  The number of axial elements
   */
  unsigned int computeNumberOfNodes(unsigned int n_elems);

  /**
   * Computes the local positions of axial nodes for an axial section
   *
   * @param[in] length  Length of the axial section
   * @param[in] n_nodes  Number of axial nodes in the axial section
   */
  std::vector<Real> getUniformNodeLocations(Real length, unsigned int n_nodes);

  /**
   * Puts local positions of axial nodes for an axial section into \c _node_locations
   *
   * @param[in] start_length  Start position for the axial section
   * @param[in] start_node  Start node index for the axial section
   * @param[in] local_node_locations  Local positions of axial nodes for the axial section
   */
  void placeLocalNodeLocations(Real start_length,
                               unsigned int start_node,
                               std::vector<Real> & local_node_locations);
};

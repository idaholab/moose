#pragma once

#include "MooseMesh.h"

class THMMesh;

template <>
InputParameters validParams<THMMesh>();

/**
 * Mesh for THM
 */
class THMMesh : public MooseMesh
{
public:
  THMMesh(const InputParameters & parameters);
  THMMesh(const THMMesh & other_mesh);

  virtual unsigned int dimension() const override;
  virtual unsigned int effectiveSpatialDimension() const override;
  virtual MooseMesh & clone() const override;
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;
  virtual void prep();

  /// Add a new node into the mesh
  Node * addNode(const Point & pt);

protected:
  /// Gets the next node ID
  virtual dof_id_type getNextNodeId();

  /// The dimension of the mesh
  unsigned int _dim;

  /// The next node ID in the mesh (used for mesh generation)
  dof_id_type _next_node_id;

public:
  static const BoundaryName INVALID_BOUNDARY_ID;
};

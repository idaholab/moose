#ifndef RELAP7MESH_H
#define RELAP7MESH_H

#include "MooseMesh.h"

class RELAP7Mesh;

template <>
InputParameters validParams<RELAP7Mesh>();

/**
 * Mesh for RELAP7
 */
class RELAP7Mesh : public MooseMesh
{
public:
  RELAP7Mesh(const InputParameters & parameters);
  RELAP7Mesh(const RELAP7Mesh & other_mesh);

  virtual unsigned int dimension() const override;
  virtual MooseMesh & clone() const override;
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;
  virtual void prep();

protected:
  /// The dimension of the mesh
  unsigned int _dim;
};

#endif /* RELAP7MESH_H */

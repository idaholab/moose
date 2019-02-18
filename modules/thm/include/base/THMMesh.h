#ifndef THMMESH_H
#define THMMESH_H

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

protected:
  /// The dimension of the mesh
  unsigned int _dim;
};

#endif /* THMMESH_H */

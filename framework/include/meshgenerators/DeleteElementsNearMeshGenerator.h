//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementDeletionGeneratorBase.h"

class KDTree;

/**
 * Deletes elements close to another mesh
 */
class DeleteElementsNearMeshGenerator : public ElementDeletionGeneratorBase
{
public:
  static InputParameters validParams();

  DeleteElementsNearMeshGenerator(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  virtual bool shouldDelete(const Elem * elem) override;

private:
  /// Mesh used to define the proximity deletion criterion
  std::unique_ptr<MeshBase> & _proximity_mesh;
  /// Point locator to weed out elements inside the proximity mesh
  std::unique_ptr<libMesh::PointLocatorBase> _pl;
  /// KD Tree to find the nearest side Qp in the proximity mesh
  std::shared_ptr<KDTree> _kd_tree;
  /// Distance for deletion criterion
  const Real _distance;
};

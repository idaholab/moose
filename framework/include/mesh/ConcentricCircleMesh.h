//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONCENTRICCIRCLEMESH_H
#define CONCENTRICCIRCLEMESH_H

#include "MooseMesh.h"
#include "MooseEnum.h"

class ConcentricCircleMesh;

template <>
InputParameters validParams<ConcentricCircleMesh>();

/**
 * Mesh generated from parameters
 */
class ConcentricCircleMesh : public MooseMesh
{
public:
  ConcentricCircleMesh(const InputParameters & parameters);
  ConcentricCircleMesh(const ConcentricCircleMesh & /* other_mesh */) = default;

  ConcentricCircleMesh & operator=(const ConcentricCircleMesh & other_mesh) = delete;
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;

protected:
  // Number of sectors in one quadrant
  unsigned int _num_sectors;
  // Radii of concentric circles
  std::vector<Real> _radii;
  // SubdomainIDs for concentric circles
  std::vector<int> _block;
  // Adding the moderator part is optional
  Real _unit_cell_length;
  // Volume preserving function is optional
  bool _volume_preserving_function;
  // Control of which portion of mesh will be developed
  MooseEnum _portion;
};

#endif /* CONCENTRICCIRCLEMESH_H */

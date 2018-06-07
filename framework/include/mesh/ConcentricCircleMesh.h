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

  // No copy
  ConcentricCircleMesh & operator=(const ConcentricCircleMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

protected:
  /// The number of sectors in one quadrant
  unsigned int _num_sectors;

  /// The number of extra intervals beyond the required box for the inner circle
  unsigned int _extra_radial_intervals;
};

#endif /* CONCENTRICCIRCLEMESH_H */

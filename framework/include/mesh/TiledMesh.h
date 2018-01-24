/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef TILEDMESH_H
#define TILEDMESH_H

#include "MooseMesh.h"

class TiledMesh;

template <>
InputParameters validParams<TiledMesh>();

class TiledMesh : public MooseMesh
{
public:
  TiledMesh(const InputParameters & parameters);
  TiledMesh(const TiledMesh & other_mesh);

  virtual MooseMesh & clone() const override;

  virtual void buildMesh() override;

protected:
  const Real _x_width;
  const Real _y_width;
  const Real _z_width;
};

#endif /* TILEDMESH_H */

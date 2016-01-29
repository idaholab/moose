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

#ifndef PERIDYNAMICSMESH_H
#define PERIDYNAMICSMESH_H

#include "MooseMesh.h"
#include "MooseEnum.h"

class PeridynamicsMesh;
class NonlinearSystem;

template<>
InputParameters validParams<PeridynamicsMesh>();

/**
 * Mesh generated from parameters
 */
class PeridynamicsMesh : public MooseMesh
{
public:
  PeridynamicsMesh(const InputParameters & parameters);
  
  virtual ~PeridynamicsMesh();

  virtual MooseMesh & clone() const;

  virtual void buildMesh();

protected:
  // The dimension of the mesh
  MooseEnum _dim;
  // Number of elements in x, y, z, R direction
 int _nx, ny, nz, _shape;
  // domain size in x, y, z, R direction
  double _xmin, _ymin, _zmin, _xmax, _ymax, _zmax, _R, mesh_spacing, horizon; 
};

#endif /* PERIDYNAMICSMESH_H */

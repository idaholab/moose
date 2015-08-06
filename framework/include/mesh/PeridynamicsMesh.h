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
  PeridynamicsMesh(const std::string & deprecated_name, InputParameters parameters); //DEPRECATED CONSTRUCTOR
  PeridynamicsMesh(const PeridynamicsMesh & other_mesh);
  virtual ~PeridynamicsMesh();

  virtual MooseMesh & clone() const;

  virtual void buildMesh();

protected:
  /// The dimension of the mesh
  MooseEnum _dim;
  /// Number of elements in x, y, z direction
  int _nx, _ny, _nz, _nr, _shape;
};

#endif /* PERIDYNAMICSMESH_H */

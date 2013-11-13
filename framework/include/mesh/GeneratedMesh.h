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

#ifndef GENERATEDMESH_H
#define GENERATEDMESH_H

#include "MooseMesh.h"

#include "MooseEnum.h"

class GeneratedMesh;
class NonlinearSystem;

template<>
InputParameters validParams<GeneratedMesh>();

/**
 * Mesh generated from parameters
 */
class GeneratedMesh : public MooseMesh
{
public:
  GeneratedMesh(const std::string & name, InputParameters parameters);
  GeneratedMesh(const GeneratedMesh & other_mesh);
  virtual ~GeneratedMesh();

  virtual MooseMesh & clone() const;

  virtual void buildMesh();

protected:
  /// The dimension of the mesh
  MooseEnum _dim;
  /// Number of elements in x, y, z direction
  int _nx, _ny, _nz;
};

#endif /* GENERATEDMESH_H */

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

#ifndef PATTERNEDMESH_H
#define PATTERNEDMESH_H

#include "MooseMesh.h"

class PatternedMesh;

template<>
InputParameters validParams<PatternedMesh>();

class PatternedMesh : public MooseMesh
{
public:
  PatternedMesh(const InputParameters & parameters);
  PatternedMesh(const PatternedMesh & other_mesh);
  ~PatternedMesh();

  virtual MooseMesh & clone() const;

  virtual void buildMesh();

protected:
  // The mesh files to read
  const std::vector<MeshFileName> & _files;

  // The pattern, starting with the upper left corner
  const std::vector<std::vector<unsigned int> > & _pattern;

  // Holds the pointers to the meshes
  std::vector<SerialMesh *> _meshes;

  // Holds a mesh for each row, these will be stitched together in the end
  std::vector<SerialMesh *> _row_meshes;

  const Real _x_width;
  const Real _y_width;
  const Real _z_width;
};

#endif /* PATTERNEDMESH_H */

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

#include "mesh.h"
#include "libmesh.h"

class MeshExtruder
{
public:
  MeshExtruder(const libMesh::MeshBase &source_mesh);

  void extrude(libMesh::MeshBase &dest_mesh, unsigned int num_layers, unsigned int axis, Real height);

private:
  const MeshBase & _src_mesh;

  static const unsigned int Quad4_to_Hex8_side_map[4];
  static const unsigned int Tri3_to_Prism6_side_map[3];
};

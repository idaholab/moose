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

#ifndef MESHEXTRUDER_H
#define MESHEXTRUDER_H

#include "MooseMesh.h"
#include "libmesh.h"

class MeshExtruder;

template<>
InputParameters validParams<MeshExtruder>();

class MeshExtruder : public MooseMesh
{
public:
//  MeshExtruder(const libMesh::MeshBase &source_mesh);
  MeshExtruder(const std::string & name, InputParameters parameters);

  void extrude(libMesh::MeshBase &dest_mesh);

protected:
  const unsigned int _num_layers;
  const Real _height;
  const unsigned int _extrusion_axis;
  libMesh::Mesh _src_mesh;

  static const unsigned int Quad4_to_Hex8_side_map[4];
  static const unsigned int Tri3_to_Prism6_side_map[3];
};

#endif /* MESHEXTRUDER_H */

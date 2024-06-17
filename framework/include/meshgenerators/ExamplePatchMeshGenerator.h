//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "MooseEnum.h"

/*
 * Mesh generator to generate 2D or 3D mesh patches
 */
class ExamplePatchMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  ExamplePatchMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /*
   *  Make quad4 elements from nodes
   *  @param mesh A reference to the mesh object
   *  @param nodes A reference to the vector containing the nodes
   */
  void makeQuad4Elems(MeshBase & mesh, const std::vector<Node *> & nodes);
  /*
   *  Make quad8 elements from nodes
   *  @param mesh A reference to the mesh object
   *  @param nodes A reference to the vector containing the nodes
   */
  void makeQuad8Elems(MeshBase & mesh, const std::vector<Node *> & nodes);
  /*
   *  Make Hex8 elements from nodes
   *  @param mesh A reference to the mesh object
   *  @param nodes A reference to the vector containing the nodes
   */
  void makeHex8Elems(MeshBase & mesh, const std::vector<Node *> & nodes);
  /*
   *  Make hex20 elements from nodes
   *  @param mesh A reference to the mesh object
   *  @param nodes A reference to the vector containing the nodes
   */
  void makeHex20Elems(MeshBase & mesh, const std::vector<Node *> & nodes);

  /// The dimension of the mesh
  MooseEnum _dim;
  /// The element type used
  MooseEnum _elem_type;
  /// Edge length of the domain in the x, y, and z directions
  Real _xlength, _ylength, _zlength;
  /// Offsets in the x, y, and z directions of the origin node (default location: (0,0,0))
  const Real _xoffset, _yoffset, _zoffset;
};

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

#include "FaceNode.h"

FaceNode::FaceNode(EFAnode* node, double xi, double eta):
  _node(node),
  _xi(xi),
  _eta(eta)
{}

FaceNode::FaceNode(const FaceNode & other_face_node):
  _node(other_face_node._node),
  _xi(other_face_node._xi),
  _eta(other_face_node._eta)
{}

FaceNode::~FaceNode()
{}

EFAnode *
FaceNode::get_node()
{
  return _node;
}

double
FaceNode::get_para_coords(unsigned int i)
{
  double coord = -100.0;
  if (i == 0)
    coord = _xi;
  else if (i == 1)
    coord = _eta;
  else
    mooseError("get_para_coords(): input out of bounds");

  return coord;
}

void
FaceNode::switchNode(EFAnode* new_node, EFAnode* old_node)
{
  if (_node == old_node)
    _node = new_node;
}

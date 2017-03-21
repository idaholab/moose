/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef FACENODE_H
#define FACENODE_H

class EFANode;

class EFAFaceNode
{
public:
  EFAFaceNode(EFANode * node, double xi, double eta);
  EFAFaceNode(const EFAFaceNode & other_face_node);

  ~EFAFaceNode();

private:
  EFANode * _node;
  double _xi;
  double _eta;

public:
  EFANode * getNode();
  double getParametricCoordinates(unsigned int i);
  void switchNode(EFANode * new_old, EFANode * old_node);
};

#endif

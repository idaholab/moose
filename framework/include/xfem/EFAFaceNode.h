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

#ifndef FACENODE_H
#define FACENODE_H

class EFANode;

class EFAFaceNode
{
public:

  EFAFaceNode(EFANode* node, double xi, double eta);
  EFAFaceNode(const EFAFaceNode & other_face_node);

  ~EFAFaceNode();

private:

  EFANode * _node;
  double _xi;
  double _eta;

public:

  EFANode * getNode();
  double getParametricCoordinates(unsigned int i);
  void switchNode(EFANode* new_old, EFANode* old_node);
};

#endif

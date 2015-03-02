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

#include "EFAnode.h"

class FaceNode
{
  public:

  FaceNode(EFAnode* node, double xi, double eta);
  FaceNode(const FaceNode & other_face_node);

  ~FaceNode();

  private:

  EFAnode * _node;
  double _xi;
  double _eta;

  public:

  EFAnode * get_node();
  double get_para_coords(unsigned int i);
  void switchNode(EFAnode* new_old, EFAnode* old_node);
};

#endif

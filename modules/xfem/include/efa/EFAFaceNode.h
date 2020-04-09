//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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

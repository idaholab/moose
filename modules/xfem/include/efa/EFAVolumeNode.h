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

class EFAVolumeNode
{
public:
  EFAVolumeNode(EFANode * node, double xi, double eta, double zeta);
  EFAVolumeNode(const EFAVolumeNode & other_vol_node);

  ~EFAVolumeNode();

private:
  EFANode * _node;
  double _xi;
  double _eta;
  double _zeta;

public:
  EFANode * getNode();
  double getParametricCoordinates(unsigned int i);
  void switchNode(EFANode * new_old, EFANode * old_node);
};

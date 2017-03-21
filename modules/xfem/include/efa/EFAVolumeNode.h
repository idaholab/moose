/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef VOLUMENODE_H
#define VOLUMENODE_H

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

#endif

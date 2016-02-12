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

#ifndef VOLUMENODE_H
#define VOLUMENODE_H

class EFANode;

class EFAVolumeNode
{
  public:

  EFAVolumeNode(EFANode* node, double xi, double eta, double zeta);
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
  void switchNode(EFANode* new_old, EFANode* old_node);
};

#endif

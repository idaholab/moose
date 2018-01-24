/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "EFAVolumeNode.h"

#include "EFANode.h"
#include "EFAError.h"

EFAVolumeNode::EFAVolumeNode(EFANode * node, double xi, double eta, double zeta)
  : _node(node), _xi(xi), _eta(eta), _zeta(zeta)
{
}

EFAVolumeNode::EFAVolumeNode(const EFAVolumeNode & other_vol_node)
  : _node(other_vol_node._node),
    _xi(other_vol_node._xi),
    _eta(other_vol_node._eta),
    _zeta(other_vol_node._zeta)
{
}

EFAVolumeNode::~EFAVolumeNode() {}

EFANode *
EFAVolumeNode::getNode()
{
  return _node;
}

double
EFAVolumeNode::getParametricCoordinates(unsigned int i)
{
  double coord = -100.0;
  if (i == 0)
    coord = _xi;
  else if (i == 1)
    coord = _eta;
  else if (i == 2)
    coord = _zeta;
  else
    EFAError("in getParametricCoordinates: input out of bounds");

  return coord;
}

void
EFAVolumeNode::switchNode(EFANode * new_node, EFANode * old_node)
{
  if (_node == old_node)
    _node = new_node;
}

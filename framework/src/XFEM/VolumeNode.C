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

#include "VolumeNode.h"

VolumeNode::VolumeNode(EFAnode* node, double xi, double eta, double zeta):
  _node(node),
  _xi(xi),
  _eta(eta),
  _zeta(zeta)
{}

VolumeNode::VolumeNode(const VolumeNode & other_vol_node):
  _node(other_vol_node._node),
  _xi(other_vol_node._xi),
  _eta(other_vol_node._eta),
  _zeta(other_vol_node._zeta)
{}

VolumeNode::~VolumeNode()
{}

EFAnode *
VolumeNode::get_node()
{
  return _node;
}

double
VolumeNode::get_para_coords(unsigned int i)
{
  double coord = -100.0;
  if (i == 0)
    coord = _xi;
  else if (i == 1)
    coord = _eta;
  else if (i == 2)
    coord = _zeta;
  else
    mooseError("get_para_coords(): input out of bounds");

  return coord;
}

void
VolumeNode::switchNode(EFAnode* new_node, EFAnode* old_node)
{
  if (_node == old_node)
    _node = new_node;
}

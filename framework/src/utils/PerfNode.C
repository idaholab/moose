//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfNode.h"

PerfNode::PerfNode(unsigned int id) : _id(id), _total_time(0) {}

PerfNode *
PerfNode::getChild(unsigned int id)
{
  auto & child_node = _children[id];

  if (!child_node)
    child_node.reset(new PerfNode(id));

  return child_node.get();
}

std::chrono::steady_clock::duration
PerfNode::selfTime()
{
  return _total_time - childrenTime();
}

std::chrono::steady_clock::duration
PerfNode::totalTime()
{
  // Note that all of the children's time is already
  // accounte for in the total time
  return _total_time;
}

std::chrono::steady_clock::duration
PerfNode::childrenTime()
{
  std::chrono::steady_clock::duration children_time(0);

  for (auto & child_it : _children)
    children_time += child_it.second->totalTime();

  return children_time;
}

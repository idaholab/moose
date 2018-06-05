//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfNode.h"

std::chrono::steady_clock::duration
PerfNode::selfTime() const
{
  return _total_time - childrenTime();
}

std::chrono::steady_clock::duration
PerfNode::totalTime() const
{
  // Note that all of the children's time is already
  // accounte for in the total time
  return _total_time;
}

std::chrono::steady_clock::duration
PerfNode::childrenTime() const
{
  std::chrono::steady_clock::duration children_time(0);

  for (auto & child_it : _children)
    children_time += child_it.second->totalTime();

  return children_time;
}

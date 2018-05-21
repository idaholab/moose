//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGuard.h"
#include "PerfGraph.h"

PerfGuard::PerfGuard(PerfGraph & graph, PerfID id) : _graph(graph)
{
  if (_graph.active())
  {
    _graph.push(id);
    _start = std::chrono::steady_clock::now();
  }
}

PerfGuard::~PerfGuard()
{
  if (_graph.active())
    _graph.pop(std::chrono::steady_clock::now() - _start);
}

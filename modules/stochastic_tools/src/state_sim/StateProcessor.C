//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StateProcessor.h"
#include <stdlib.h>
#include <string>
#include <cmath>
#include "MooseRandom.h"

StateProcessor::StateProcessor(unsigned int max_time_step, int seed)
  : _max_time_step(max_time_step), _ev_times()
{
  MooseRandom::seed(seed);

  // add a bunch of random time items
  unsigned int r = MooseRandom::randl() % 10;
  for (unsigned int i = 0; i < r; i++)
  {
    unsigned int addI = MooseRandom::randl() % max_time_step;
    addEv(addI);
  }
}

// Add the event in to the list in the proper order
void
StateProcessor::addEv(unsigned int time_step = 0)
{
  unsigned long idx = 0;
  while ((idx < _ev_times.size()) && (_ev_times[idx] > time_step))
    ++idx;

  _ev_times.insert(_ev_times.begin() + idx, time_step);
}

unsigned int
StateProcessor::nextTime()
{
  if (_ev_times.size() > 0)
    return _ev_times.back();
  else
    return 0;
}

unsigned int
StateProcessor::process(unsigned int time_step)
{
  if (time_step == _ev_times.back())
    _ev_times.pop_back();

  // todo run state processing

  if (_ev_times.size() > 0)
    return _ev_times.back();
  else
    return 0;
}

void
StateProcessor::setMaxTime(unsigned time_step)
{
  _max_time_step = time_step;
}

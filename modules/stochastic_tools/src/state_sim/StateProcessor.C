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
#include "StateProcessor.h"
#include <stdlib.h>
#include <string>
#include <cmath>
//#include "StateSimBase.h"

using namespace std;

StateProcessor::StateProcessor(int max_time_step)
  : _max_time_step(max_time_step),
    _ev_times()
{

  //add a bunch of random time items
  int r = rand() % 10;
  for (int i = 0; i < r; i++)
  {
    int addI = rand() % max_time_step;
    addEv(addI);
  }
}

//Add the event in to the list in the proper order
void
StateProcessor::addEv(int time_step = 0)
{
  unsigned long idx = 0;
  while ((idx < _ev_times.size()) && (_ev_times[idx] > time_step))
    ++idx;

  _ev_times.insert(_ev_times.begin() + idx, time_step);
}

int
StateProcessor::nextTime()
{
  //StateSimBase test;
  if (_ev_times.size() > 0)
    return _ev_times.back();
  else
    return 0;
}

int
StateProcessor::process(int time_step)
{
  if (time_step == _ev_times.back())
    _ev_times.pop_back();

  //todo run state processing

  if (_ev_times.size() > 0)
    return _ev_times.back();
  else
    return 0;
}

void
StateProcessor::setMaxTime(int time_step)
{
  _max_time_step = time_step;
}

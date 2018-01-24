//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STATEPROCESSOR_H
#define STATEPROCESSOR_H

#include <string>
#include <vector>

/**
 Object to load and run and manage a state simulation model
 */
class StateProcessor
{
public:
  // with default value
  StateProcessor(unsigned int max_time_step, int seed = 0);
  void setMaxTime(unsigned int time_step);

  unsigned int nextTime();
  unsigned int process(unsigned int time_step);

private:
  unsigned int _max_time_step;
  std::vector<unsigned int> _ev_times;

  void addEv(unsigned int time_step);
};

#endif

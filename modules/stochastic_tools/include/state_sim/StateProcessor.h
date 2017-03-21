/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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

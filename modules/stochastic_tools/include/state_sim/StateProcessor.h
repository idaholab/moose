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
#ifndef STATEPROCESSOR_H
#define STATEPROCESSOR_H

#include <string>
#include <vector>

using namespace std;

class StateProcessor
{
  private:
    int _max_time_step;
    std::vector<int> _ev_times;

    void addEv(int time_step);

  public:
    //with default value
    StateProcessor(int max_time_step);
    void setMaxTime(int time_step);

    int nextTime();
    int process(int time_step);
};

#endif

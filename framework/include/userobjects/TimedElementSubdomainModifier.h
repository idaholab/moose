//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementSubdomainModifier.h"
#include "DelimitedFileReaderOfString.h"

class TimedElementSubdomainModifier : public ElementSubdomainModifier
{
public:
  static InputParameters validParams();

  TimedElementSubdomainModifier(const InputParameters & parameters);

  virtual void initialize();
  // virtual void execute() {}
  // virtual void finalize() {}

protected:
  virtual SubdomainID computeSubdomainID() override;

  virtual std::vector<double> onGetTimes();
  virtual SubdomainID onComputeSubdomainID(double t_from_exclusive, double t_to_inclusive);

  /// storage for the times including their original index.
  struct timeIndexPair
  {
    double time;
    std::size_t index;

    bool operator<(const timeIndexPair & a) const
    {
      if (time == a.time)
      {
        return index < a.index;
      }
      else
      {
        return time < a.time;
      };
    }
  };

  std::vector<timeIndexPair> _timesAndIndices;

private:
  // some state variables
  int _current_step;
  double _current_t;
  double _last_t;
};

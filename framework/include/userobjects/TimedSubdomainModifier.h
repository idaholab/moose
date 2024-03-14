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
#include "TimedElementSubdomainModifier.h"
#include "DelimitedFileReaderOfString.h"

class TimedSubdomainModifier : public TimedElementSubdomainModifier
{
public:
  static InputParameters validParams();

  TimedSubdomainModifier(const InputParameters & parameters);

  virtual void initialize();

protected:
  virtual std::vector<double> onGetTimes() override;
  virtual SubdomainID onComputeSubdomainID(double t_from_exclusive, double t_to_inclusive) override;

private:

  void buildFromParameters();
  void buildFromFile();

  std::vector<double> _times;

  /// storage for the block ids.
  std::vector<SubdomainID> _blocks_from;
  std::vector<SubdomainID> _blocks_to;

};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SidePostprocessor.h"

// Forward Declarations
class AverageBID;

template <>
InputParameters validParams<AverageBID>();

class AverageBID : public SidePostprocessor
{
public:
  AverageBID(const InputParameters & parameters);

  virtual PostprocessorValue getValue() override;
  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  Real _sum_bid;
  unsigned int _n_summands;
};

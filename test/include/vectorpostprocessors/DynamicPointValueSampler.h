//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "LineValueSampler.h"

// Forward Declarations
class DynamicPointValueSampler;

template <>
InputParameters validParams<DynamicPointValueSampler>();

class DynamicPointValueSampler : public LineValueSampler
{
public:
  DynamicPointValueSampler(const InputParameters & parameters);

  virtual void initialize() override;

protected:
  const unsigned int _adder;

  const bool _use_transfer;
};


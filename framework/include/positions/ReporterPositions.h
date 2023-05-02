//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Positions.h"

/**
 * Positions from a Reporter
 */
class ReporterPositions : public Positions
{
public:
  static InputParameters validParams();
  ReporterPositions(const InputParameters & parameters);
  virtual ~ReporterPositions() = default;

  void initialize() override;
};

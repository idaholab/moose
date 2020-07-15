//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPolyLineSink.h"
#include "GeneralVectorPostprocessor.h"

/**
 * Approximates a line sink by a sequence of Dirac Points
 */
class DiracPointsWriter : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  DiracPointsWriter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;

protected:
  VectorPostprocessorValue & _xs;
  VectorPostprocessorValue & _ys;
  VectorPostprocessorValue & _zs;
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "NearestNodeNumberUO.h"

/**
 * Given a NearestNodeNumberUO, outputs the nearest node number to a given point
 */
class NearestNodeNumber : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  NearestNodeNumber(const InputParameters & parameters);

  virtual void execute() override{};
  virtual void initialize() override{};
  virtual PostprocessorValue getValue() override;

private:
  /// The nearest node number UserObject that does all the work
  const NearestNodeNumberUO & _nnn;
};

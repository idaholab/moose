//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Test class to make certain that CSV data is broadcast correctly.
 */
class TestCSVReader : public GeneralUserObject
{
public:
  static InputParameters validParams();

  TestCSVReader(const InputParameters & parameters);
  virtual void execute() override;
  virtual void initialize() override {}
  virtual void finalize() override {}

protected:
  const VectorPostprocessorValue & _vpp_data;
  const processor_id_type & _rank;
  const std::vector<double> & _gold;
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

class GriddedDataReporter : public GeneralReporter
{
public:
  static InputParameters validParams();

  GriddedDataReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

protected:
  FileName _file_name;
  std::vector<Real> & _parameters;
  std::vector<std::vector<Real>> & _grid;
  std::vector<int> & _axes;
  std::vector<unsigned int> & _step;
  unsigned int & _dim;
};

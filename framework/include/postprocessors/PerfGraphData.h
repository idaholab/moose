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

class PerfGraphData : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  PerfGraphData(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual Real getValue() override;

protected:
  /// The data type to request in regards to the PerfGraph section
  const int _data_type;

  /// The section name in the PerfGraph to query
  const std::string & _section_name;

  /// Whether or not the section must exist (if it does not, 0 is returned)
  const bool _must_exist;

private:
  /// The current data
  Real _current_data;
};

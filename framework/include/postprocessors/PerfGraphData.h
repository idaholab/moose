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

// Forward Declarations
class PerfGraphData;

// libMesh forward declarations
namespace libMesh
{
class System;
class EquationSystems;
}

template <>
InputParameters validParams<PerfGraphData>();

class PerfGraphData : public GeneralPostprocessor
{
public:
  PerfGraphData(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual Real getValue() override;

protected:
  const int _data_type;

  const std::string & _section_name;
};


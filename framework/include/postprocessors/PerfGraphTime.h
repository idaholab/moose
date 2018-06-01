//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PERFGRAPHTIME_H
#define PERFGRAPHTIME_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class PerfGraphTime;

// libMesh forward declarations
namespace libMesh
{
class System;
class EquationSystems;
}

template <>
InputParameters validParams<PerfGraphTime>();

class PerfGraphTime : public GeneralPostprocessor
{
public:
  PerfGraphTime(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual Real getValue() override;

protected:
  const Real & _time;
};

#endif // PERFGRAPHTIME_H

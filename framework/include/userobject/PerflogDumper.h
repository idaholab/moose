//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PERFLOGDUMPER_H
#define PERFLOGDUMPER_H

#include "GeneralUserObject.h"

class PerflogDumper;

template <>
InputParameters validParams<PerflogDumper>();

/// Records all post processor data in a CSV file.
class PerflogDumper : public GeneralUserObject
{
public:
  PerflogDumper(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override{};
};

#endif

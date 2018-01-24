//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CAVITYPRESSUREPOSTPROCESSOR_H
#define CAVITYPRESSUREPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class CavityPressureUserObject;

class CavityPressurePostprocessor : public GeneralPostprocessor
{
public:
  CavityPressurePostprocessor(const InputParameters & parameters);

  virtual ~CavityPressurePostprocessor() {}

  virtual void initialize() {}

  virtual void execute() {}

  virtual PostprocessorValue getValue();

protected:
  const CavityPressureUserObject & _cpuo;

  const std::string _quantity;
};

template <>
InputParameters validParams<CavityPressurePostprocessor>();

#endif

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONSTANTVECTORPOSTPROCESSOR_H
#define CONSTANTVECTORPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class ConstantVectorPostprocessor;

template <>
InputParameters validParams<ConstantVectorPostprocessor>();

class ConstantVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  ConstantVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

protected:
  VectorPostprocessorValue & _value;
};

#endif

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LATEDECLARATIONVECTORPOSTPROCESSOR_H
#define LATEDECLARATIONVECTORPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class LateDeclarationVectorPostprocessor;

template <>
InputParameters validParams<LateDeclarationVectorPostprocessor>();

class LateDeclarationVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  LateDeclarationVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

protected:
  VectorPostprocessorValue * _value;
};

#endif

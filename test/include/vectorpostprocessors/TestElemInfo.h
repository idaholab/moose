//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"
#include "Coupleable.h"

// Forward Declarations
class MooseMesh;

class TestElemInfo : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  TestElemInfo(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;

protected:
  VectorPostprocessorValue & _elem_id;
  VectorPostprocessorValue & _elem_volume;
  VectorPostprocessorValue & _cx;
  VectorPostprocessorValue & _cy;
  VectorPostprocessorValue & _cz;

  std::vector<VariableName> _vars;
  std::vector<VectorPostprocessorValue *> _var_dof_indices;
};

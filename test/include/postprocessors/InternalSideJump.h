//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSidePostprocessor.h"

class InternalSideJump : public InternalSidePostprocessor
{
public:
  static InputParameters validParams();

  InternalSideJump(const InputParameters & parameters);

  virtual PostprocessorValue getValue() override;
  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  const Real & _current_elem_volume;
  const Real & _current_neighbor_volume;
  const VariableValue & _sln_dofs;
  const VariableValue & _sln_dofs_neig;
  Real _integral_value;
};
